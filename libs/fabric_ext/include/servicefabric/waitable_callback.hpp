#pragma once
#include "FabricCommon.h"

#include <condition_variable>
#include <mutex>

#include <moderncom/interfaces.h>

namespace servicefabric {

MIDL_INTERFACE("b8fa8b9b-c874-407a-99fe-f3b0aa0ba5e7")
IFabricAsyncOperationWaitableCallback : public IFabricAsyncOperationCallback {
public:
  // wait for operation to be called
  virtual void STDMETHODCALLTYPE Wait() = 0;
};

class FabricAsyncOperationWaitableCallback
    : public belt::com::object<
          FabricAsyncOperationWaitableCallback,  // our class
          IFabricAsyncOperationWaitableCallback> // we implement
{

public:
  FabricAsyncOperationWaitableCallback();
  void Invoke(/* [in] */ IFabricAsyncOperationContext *context) override;

  // wait for cv notify
  void Wait() override;

private:
  std::mutex m_;
  std::condition_variable cv_;
  bool ready_;
};

} // namespace servicefabric