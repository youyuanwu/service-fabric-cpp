// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "service_factory.hpp"
#include "app_instance.hpp"
#include <boost/log/trivial.hpp>

service_factory::service_factory(ULONG port, std::wstring hostname)
    : port_(port), hostname_(hostname) {}

HRESULT STDMETHODCALLTYPE service_factory::CreateInstance(
    /* [in] */ LPCWSTR serviceTypeName,
    /* [in] */ FABRIC_URI serviceName,
    /* [in] */ ULONG initializationDataLength,
    /* [size_is][in] */ const byte *initializationData,
    /* [in] */ FABRIC_PARTITION_ID partitionId,
    /* [in] */ FABRIC_INSTANCE_ID instanceId,
    /* [retval][out] */ IFabricStatelessServiceInstance **serviceInstance) {

  std::string data;
  if (initializationDataLength > 0 && initializationData != nullptr) {
    data = std::string(initializationData,
                       initializationData + initializationDataLength);
  }

  UNREFERENCED_PARAMETER(partitionId);

  BOOST_LOG_TRIVIAL(debug) << "service_factory::CreateInstance "
                           << "serviceTypeName " << serviceTypeName
                           << "serviceName " << serviceName
                           << "initializationDataLength "
                           << initializationDataLength << "initializationData"
                           << data
                           // << "partitionId " << partitionId
                           << "instanceId " << instanceId;

  // create a instance and return
  winrt::com_ptr<IFabricStatelessServiceInstance> instance =
      winrt::make<app_instance>(port_, hostname_);

  *serviceInstance = instance.detach();
  return NO_ERROR;
}