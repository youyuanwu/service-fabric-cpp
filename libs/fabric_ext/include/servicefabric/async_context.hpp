#pragma once

#include "FabricCommon.h"
// #include "FabricRuntime.h"
#include <moderncom/interfaces.h>

namespace servicefabric {

// a dummy context which always completes sync
class async_context
    : public belt::com::object<async_context, IFabricAsyncOperationContext> {
public:
  async_context(IFabricAsyncOperationCallback *callback);
  /*IFabricAsyncOperationContext members*/
  BOOLEAN STDMETHODCALLTYPE IsCompleted() override;
  BOOLEAN STDMETHODCALLTYPE CompletedSynchronously() override;
  HRESULT STDMETHODCALLTYPE get_Callback(
      /* [retval][out] */ IFabricAsyncOperationCallback **callback) override;
  HRESULT STDMETHODCALLTYPE Cancel() override;

private:
  belt::com::com_ptr<IFabricAsyncOperationCallback> callback_;
};

} // namespace servicefabric