#include "servicefabric/waitable_callback.hpp"
#include <boost/log/trivial.hpp>

namespace servicefabric {

FabricAsyncOperationWaitableCallback::FabricAsyncOperationWaitableCallback()
    : m_(), cv_(), ready_(false) {}

void FabricAsyncOperationWaitableCallback::Invoke(
    /* [in] */ IFabricAsyncOperationContext *context) {
  {
    UNREFERENCED_PARAMETER(context);
    std::lock_guard lk(m_);
    ready_ = true;
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "FabricAsyncOperationWaitableCallback::Invoke";
#endif
  }
  cv_.notify_all();
}

void FabricAsyncOperationWaitableCallback::Wait() {
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "FabricAsyncOperationWaitableCallback::Wait";
#endif
  std::unique_lock lk(m_);
  cv_.wait(lk, [this] { return ready_; });
}

} // namespace servicefabric