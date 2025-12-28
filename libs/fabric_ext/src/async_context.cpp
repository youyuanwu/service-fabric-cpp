// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "servicefabric/async_context.hpp"
#include <spdlog/spdlog.h>

namespace servicefabric {

async_context::async_context(IFabricAsyncOperationCallback *callback)
    : callback_() {
  callback_.copy_from(callback);
  spdlog::debug("async_context::async_context");
  // do not store callback works

  // invoke callback
  callback_->Invoke(this);
}

BOOLEAN STDMETHODCALLTYPE async_context::IsCompleted() {
  spdlog::debug("async_context::IsCompleted");
  return true;
}
BOOLEAN STDMETHODCALLTYPE async_context::CompletedSynchronously() {
  spdlog::debug("async_context::CompletedSynchronously");
  return true;
}
HRESULT STDMETHODCALLTYPE async_context::get_Callback(
    /* [retval][out] */ IFabricAsyncOperationCallback **callback) {
  spdlog::debug("async_context::get_Callback");
  // callback is return as a reference.
  *callback = callback_.get();
  return S_OK;
}
HRESULT STDMETHODCALLTYPE async_context::Cancel() {
  spdlog::debug("async_context::Cancel");
  return S_OK;
}

} // namespace servicefabric