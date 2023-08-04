// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "FabricCommon.h"
#include "FabricRuntime.h"
#include <winrt/base.h>

class service_factory
    : public winrt::implements<service_factory,
                               IFabricStatelessServiceFactory> {

public:
  service_factory(ULONG port, std::wstring hostname);

  HRESULT STDMETHODCALLTYPE CreateInstance(
      /* [in] */ LPCWSTR serviceTypeName,
      /* [in] */ FABRIC_URI serviceName,
      /* [in] */ ULONG initializationDataLength,
      /* [size_is][in] */ const byte *initializationData,
      /* [in] */ FABRIC_PARTITION_ID partitionId,
      /* [in] */ FABRIC_INSTANCE_ID instanceId,
      /* [retval][out] */ IFabricStatelessServiceInstance **serviceInstance)
      override;

private:
  ULONG port_;
  std::wstring hostname_;
};