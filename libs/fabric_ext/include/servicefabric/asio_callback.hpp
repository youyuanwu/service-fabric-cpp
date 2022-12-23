#pragma once

#include "FabricCommon.h"
#include <boost/asio/windows/overlapped_ptr.hpp>
#include <moderncom/interfaces.h>

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
    : public belt::com::object<AsioCallback, IFabricAsyncOperationCallback> {
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

} // namespace servicefabric