#pragma once

#include "FabricCommon.h"
#include "FabricRuntime.h"
#include <moderncom/interfaces.h>

class service_factory: public belt::com::object<
    service_factory,
    IFabricStatelessServiceFactory>{
    
public:
    service_factory(ULONG port, std::wstring hostname);

    HRESULT STDMETHODCALLTYPE CreateInstance( 
        /* [in] */ LPCWSTR serviceTypeName,
        /* [in] */ FABRIC_URI serviceName,
        /* [in] */ ULONG initializationDataLength,
        /* [size_is][in] */ const byte *initializationData,
        /* [in] */ FABRIC_PARTITION_ID partitionId,
        /* [in] */ FABRIC_INSTANCE_ID instanceId,
        /* [retval][out] */ IFabricStatelessServiceInstance **serviceInstance) override;
private:
    ULONG port_;
    std::wstring hostname_;
};