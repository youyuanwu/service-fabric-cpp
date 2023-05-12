// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "app_instance.hpp"
#include "servicefabric/async_context.hpp"
#include "servicefabric/string_result.hpp"
#include <boost/log/trivial.hpp>

namespace sf = servicefabric;

app_instance::app_instance(ULONG port, std::wstring hostname)
    : server_(nullptr), io_context_(), port_(port), hostname_(hostname),
      server_threads_() {
  BOOST_LOG_TRIVIAL(debug) << "app_instance::app_instance"
                           << " hostname " << hostname_ << " port " << port_;
}

HRESULT STDMETHODCALLTYPE app_instance::BeginOpen(
    /* [in] */ IFabricStatelessServicePartition *partition,
    /* [in] */ IFabricAsyncOperationCallback *callback,
    /* [retval][out] */ IFabricAsyncOperationContext **context) {

  BOOST_LOG_TRIVIAL(debug) << "app_instance::BeginOpen";
  if (partition == nullptr || callback == nullptr || context == nullptr) {
    BOOST_LOG_TRIVIAL(debug) << "nullptr";
    return E_POINTER;
  }
  // make server
  server_ = std::make_unique<server>(io_context_, static_cast<short>(port_));
  BOOST_LOG_TRIVIAL(debug) << "start server in background";
  for (int n = 0; n < 2; ++n) {
    server_threads_.emplace_back([this] { io_context_.run(); });
  }

  belt::com::com_ptr<IFabricAsyncOperationContext> ctx =
      sf::async_context::create_instance(callback).to_ptr();

  *context = ctx.detach();
  return S_OK;
}

HRESULT STDMETHODCALLTYPE app_instance::EndOpen(
    /* [in] */ IFabricAsyncOperationContext *context,
    /* [retval][out] */ IFabricStringResult **serviceAddress) {
  BOOST_LOG_TRIVIAL(debug) << "app_instance::EndOpen";
  if (context == nullptr || serviceAddress == nullptr) {
    BOOST_LOG_TRIVIAL(debug) << "nullptr";
    return E_POINTER;
  }

  std::wstring addr_str = hostname_ + L":" + std::to_wstring(port_);

  belt::com::com_ptr<IFabricStringResult> addr =
      sf::string_result::create_instance(addr_str).to_ptr();
  *serviceAddress = addr.detach();

  return S_OK;
}

HRESULT STDMETHODCALLTYPE app_instance::BeginClose(
    /* [in] */ IFabricAsyncOperationCallback *callback,
    /* [retval][out] */ IFabricAsyncOperationContext **context) {
  BOOST_LOG_TRIVIAL(debug) << "app_instance::BeginClose";
  if (callback == nullptr || context == nullptr) {
    BOOST_LOG_TRIVIAL(debug) << "nullptr";
    return E_POINTER;
  }

  io_context_.stop();
  for (auto &thread : server_threads_) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  belt::com::com_ptr<IFabricAsyncOperationContext> ctx =
      sf::async_context::create_instance(callback).to_ptr();

  *context = ctx.detach();

  return S_OK;
}

HRESULT STDMETHODCALLTYPE app_instance::EndClose(
    /* [in] */ IFabricAsyncOperationContext *context) {
  BOOST_LOG_TRIVIAL(debug) << "app_instance::EndClose";
  if (context == nullptr) {
    BOOST_LOG_TRIVIAL(debug) << "nullptr";
    return E_POINTER;
  }
  return S_OK;
}

void STDMETHODCALLTYPE app_instance::Abort(void) {
  BOOST_LOG_TRIVIAL(debug) << "app_instance::Abort";
}