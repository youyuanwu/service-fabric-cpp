// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "AsyncOperation.h"
#include "ComPointer.h"
#include "ComUnknownBase.h"
#include "FabricCommon.h"
#include "stdafx.h"

namespace Common {
class ComProxyAsyncOperation : public AsyncOperation {
  DENY_COPY(ComProxyAsyncOperation)

public:
  virtual ~ComProxyAsyncOperation();

protected:
  ComProxyAsyncOperation(AsyncCallback const &callback,
                         AsyncOperationSPtr const &parent,
                         bool skipCompleteOnCancel = false);

  virtual HRESULT
  BeginComAsyncOperation(IFabricAsyncOperationCallback *callback,
                         IFabricAsyncOperationContext **context) = 0;
  virtual HRESULT
  EndComAsyncOperation(IFabricAsyncOperationContext *context) = 0;

  void OnStart(AsyncOperationSPtr const &thisSPtr);
  void Cleanup();
  virtual void OnCancel();

private:
  HRESULT OnComAsyncOperationCompleted(AsyncOperationSPtr const &thisSPtr,
                                       IFabricAsyncOperationContext *context);
  HRESULT FinishComAsyncOperation(AsyncOperationSPtr const &thisSPtr,
                                  IFabricAsyncOperationContext *context);
  HRESULT TryComplete_HRESULT(AsyncOperationSPtr const &thisSPtr, HRESULT hr,
                              bool captureThreadErrorMessage);

private:
  std::shared_ptr<ComPointer<IFabricAsyncOperationContext>>
      comAsyncOperationSPtr_;
  // Common::ExclusiveLock lock_;
  std::mutex lock_;
  class ComAsyncOperationCallback;
  friend class ComAsyncOperationCallback;
};
} // namespace Common
