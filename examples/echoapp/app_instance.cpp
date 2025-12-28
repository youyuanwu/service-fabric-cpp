// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "app_instance.hpp"
#include "servicefabric/async_context.hpp"
#include "servicefabric/string_result.hpp"
#include <spdlog/spdlog.h>

namespace sf = servicefabric;

app_instance::app_instance(ULONG port, std::wstring hostname)
    : server_(nullptr), io_context_(), port_(port), hostname_(hostname),
      server_threads_() {
  spdlog::debug(L"app_instance::app_instance hostname {} port {}", hostname_,
                port_);
}

HRESULT STDMETHODCALLTYPE app_instance::BeginOpen(
    /* [in] */ IFabricStatelessServicePartition *partition,
    /* [in] */ IFabricAsyncOperationCallback *callback,
    /* [retval][out] */ IFabricAsyncOperationContext **context) {

  spdlog::debug("app_instance::BeginOpen");
  if (partition == nullptr || callback == nullptr || context == nullptr) {
    spdlog::debug("nullptr");
    return E_POINTER;
  }
  // make server
  server_ = std::make_unique<server>(io_context_, static_cast<short>(port_));
  spdlog::debug("start server in background");
  for (int n = 0; n < 2; ++n) {
    server_threads_.emplace_back([this] { io_context_.run(); });
  }

  winrt::com_ptr<IFabricAsyncOperationContext> ctx =
      winrt::make<sf::async_context>(callback);

  *context = ctx.detach();
  return S_OK;
}

HRESULT STDMETHODCALLTYPE app_instance::EndOpen(
    /* [in] */ IFabricAsyncOperationContext *context,
    /* [retval][out] */ IFabricStringResult **serviceAddress) {
  spdlog::debug("app_instance::EndOpen");
  if (context == nullptr || serviceAddress == nullptr) {
    spdlog::debug("nullptr");
    return E_POINTER;
  }

  std::wstring addr_str = hostname_ + L":" + std::to_wstring(port_);

  winrt::com_ptr<IFabricStringResult> addr =
      winrt::make<sf::string_result>(addr_str);
  *serviceAddress = addr.detach();

  return S_OK;
}

HRESULT STDMETHODCALLTYPE app_instance::BeginClose(
    /* [in] */ IFabricAsyncOperationCallback *callback,
    /* [retval][out] */ IFabricAsyncOperationContext **context) {
  spdlog::debug("app_instance::BeginClose");
  if (callback == nullptr || context == nullptr) {
    spdlog::debug("nullptr");
    return E_POINTER;
  }

  io_context_.stop();
  for (auto &thread : server_threads_) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  winrt::com_ptr<IFabricAsyncOperationContext> ctx =
      winrt::make<sf::async_context>(callback);

  *context = ctx.detach();

  return S_OK;
}

HRESULT STDMETHODCALLTYPE app_instance::EndClose(
    /* [in] */ IFabricAsyncOperationContext *context) {
  spdlog::debug("app_instance::EndClose");
  if (context == nullptr) {
    spdlog::debug("nullptr");
    return E_POINTER;
  }
  return S_OK;
}

void STDMETHODCALLTYPE app_instance::Abort(void) {
  spdlog::debug("app_instance::Abort");
}