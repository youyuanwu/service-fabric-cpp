// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "FabricCommon.h"
#include <winrt/base.h>

namespace servicefabric {

// a dummy context which always completes sync
class async_context
    : public winrt::implements<async_context, IFabricAsyncOperationContext> {
public:
  async_context(IFabricAsyncOperationCallback *callback);
  /*IFabricAsyncOperationContext members*/
  BOOLEAN STDMETHODCALLTYPE IsCompleted() override;
  BOOLEAN STDMETHODCALLTYPE CompletedSynchronously() override;
  HRESULT STDMETHODCALLTYPE get_Callback(
      /* [retval][out] */ IFabricAsyncOperationCallback **callback) override;
  HRESULT STDMETHODCALLTYPE Cancel() override;

private:
  winrt::com_ptr<IFabricAsyncOperationCallback> callback_;
};

} // namespace servicefabric