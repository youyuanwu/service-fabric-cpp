// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "servicefabric/waitable_callback.hpp"
#include <spdlog/spdlog.h>

namespace servicefabric {

FabricAsyncOperationWaitableCallback::FabricAsyncOperationWaitableCallback()
    : m_(), cv_(), ready_(false) {}

void FabricAsyncOperationWaitableCallback::Invoke(
    /* [in] */ IFabricAsyncOperationContext *context) {
  {
    UNREFERENCED_PARAMETER(context);
    std::lock_guard lk(m_);
    ready_ = true;
    spdlog::debug("FabricAsyncOperationWaitableCallback::Invoke");
  }
  cv_.notify_all();
}

void FabricAsyncOperationWaitableCallback::Wait() {
  spdlog::debug("FabricAsyncOperationWaitableCallback::Wait");
  std::unique_lock lk(m_);
  cv_.wait(lk, [this] { return ready_; });
}

} // namespace servicefabric