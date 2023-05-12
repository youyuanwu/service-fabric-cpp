// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "servicefabric/async_context.hpp"
#include <boost/log/trivial.hpp>

namespace servicefabric {

async_context::async_context(IFabricAsyncOperationCallback *callback)
    : callback_(callback) {
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "async_context::async_context";
#endif
  // do not store callback works

  // invoke callback
  callback_->Invoke(this);
}

BOOLEAN STDMETHODCALLTYPE async_context::IsCompleted() {
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "async_context::IsCompleted";
#endif
  return true;
}
BOOLEAN STDMETHODCALLTYPE async_context::CompletedSynchronously() {
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "async_context::CompletedSynchronously";
#endif
  return true;
}
HRESULT STDMETHODCALLTYPE async_context::get_Callback(
    /* [retval][out] */ IFabricAsyncOperationCallback **callback) {
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "async_context::get_Callback";
#endif
  // callback is return as a reference.
  *callback = callback_.get();
  return S_OK;
}
HRESULT STDMETHODCALLTYPE async_context::Cancel() {
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "async_context::Cancel";
#endif
  return S_OK;
}

} // namespace servicefabric