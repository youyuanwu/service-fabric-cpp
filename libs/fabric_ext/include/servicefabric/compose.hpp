// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "servicefabric/asio_callback.hpp"

#include <spdlog/spdlog.h>

namespace servicefabric {

// compose can be used to make begin end func composition
template <typename Executor, typename Client, typename BeginFunc,
          typename EndFunc>
class compose_op {
public:
  compose_op(Executor ex, winrt::com_ptr<Client> client, BeginFunc bf,
             EndFunc ef)
      : ex_(ex), client_(client), bf_(bf), ef_(ef), callback_(), ctx_() {}

  // Query is the type input to Begin func
  // token result is the type input to End func
  template <typename Query, typename Result>
  void async_exec(Query *q,
                  std::function<void(asio::error_code, Result *)> token) {
    ASIO_ASSERT(bf_ != nullptr);
    ASIO_ASSERT(ef_ != nullptr);
    ASIO_ASSERT(client_.get() != nullptr);

    // construct callback
    auto lamda_callback = [this, token](IFabricAsyncOperationContext *ctx) {
      asio::error_code ec;
      winrt::com_ptr<Result> result;
      // invoke end func
      HRESULT hr = (client_.get()->*ef_)(ctx, result.put());
      if (hr != NO_ERROR) {
        ec = asio::error_code(hr, asio::error::get_system_category());
        spdlog::debug("EndOperation failed: {}", ec.message());
      }
      token(ec, result.get());
    };

    callback_ = winrt::make<AsioCallback>(lamda_callback, ex_);
    // invoke begin func
    HRESULT hr = (client_.get()->*bf_)(q, 1000, callback_.get(), ctx_.put());
    if (hr != NO_ERROR) {
      auto ec = asio::error_code(hr, asio::error::get_system_category());
      spdlog::debug("BeginOperation failed: {}", ec.message());
      // begin failed we need to clean the asio callback?
      ASIO_ASSERT(false);
    }
  }

private:
  // operation signature types
  winrt::com_ptr<Client> client_;
  BeginFunc bf_;
  EndFunc ef_;
  Executor ex_;

  // self owned internal
  winrt::com_ptr<IFabricAsyncOperationCallback> callback_;
  winrt::com_ptr<IFabricAsyncOperationContext> ctx_;
};

} // namespace servicefabric