// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "FabricCommon.h"
#include <boost/asio/windows/object_handle.hpp>
#include <boost/asio/windows/overlapped_ptr.hpp>

// #include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <winrt/base.h>

#include <functional>

namespace servicefabric {

namespace details {

class callback_obj {
public:
  template <typename Executor>
  callback_obj(std::function<void(IFabricAsyncOperationContext *)> token,
               Executor ex)
      : optr_(), ctx_(nullptr), token_(token) {
    optr_.reset(ex,
                [this]([[maybe_unused]] boost::system::error_code ec, size_t) {
                  // invoke the callback.
                  BOOST_ASSERT(!ec.failed());
                  token_(ctx_);
                });
  }

  void complete() {
    BOOST_ASSERT(token_ != nullptr);
    // BOOST_ASSERT(optr.)
    BOOST_ASSERT(ctx_ != nullptr);
    boost::system::error_code ec;
    optr_.complete(ec, 0);
  }

  // set the reference
  void set_ctx(IFabricAsyncOperationContext *context) { ctx_ = context; }

private:
  boost::asio::windows::overlapped_ptr optr_;
  std::function<void(IFabricAsyncOperationContext *)> token_;
  IFabricAsyncOperationContext *ctx_;
};

} // namespace details

class AsioCallback
    : public winrt::implements<AsioCallback, IFabricAsyncOperationCallback> {
public:
  template <typename Executor>
  AsioCallback(std::function<void(IFabricAsyncOperationContext *)> token,
               Executor ex)
      : c_obj_(token, ex) {}

  void Invoke(/* [in] */ IFabricAsyncOperationContext *context) override {
    c_obj_.set_ctx(context);
    c_obj_.complete();
  }

private:
  details::callback_obj c_obj_;
};

MIDL_INTERFACE("2ebd9df8-f94b-4ab5-bfd4-13af87298f3d")
IAwaitableCallback : public IFabricAsyncOperationCallback {
public:
  // for co_await
  virtual boost::asio::awaitable<void> await() = 0;
};

class AsioAwaitableCallback
    : public winrt::implements<AsioAwaitableCallback, IAwaitableCallback> {
public:
  template <typename Executor> AsioAwaitableCallback(Executor ex) : oh_(ex) {
    // create a event and assgn to handle
    HANDLE ev = CreateEvent(NULL,  // default security attributes
                            TRUE,  // manual-reset event
                            FALSE, // initial state is nonsignaled
                            NULL   // object name
    );
    assert(ev != nullptr);
    oh_.assign(ev);
  }

  void Invoke(/* [in] */ IFabricAsyncOperationContext *context) override {
    UNREFERENCED_PARAMETER(context);
    HANDLE ev = oh_.native_handle();
    assert(ev != nullptr);
    [[maybe_unused]] bool ok = SetEvent(ev);
    assert(ok);
  }

  boost::asio::awaitable<void> await() override {
    assert(oh_.is_open());
    return oh_.async_wait(boost::asio::use_awaitable);
  }

private:
  // boost::asio::windows::overlapped_ptr optr_;
  boost::asio::windows::object_handle oh_;
};

} // namespace servicefabric