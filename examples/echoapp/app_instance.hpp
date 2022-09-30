#pragma once

#include "FabricCommon.h"
#include "FabricRuntime.h"
#include <moderncom/interfaces.h>

#include "echo_server.h"

#include <memory>
#include <vector>
#include <thread>
#include <boost/asio.hpp>


class app_instance : public belt::com::object<
    app_instance,
    IFabricStatelessServiceInstance>{
public:
    app_instance(ULONG port, std::wstring hostname);

    HRESULT STDMETHODCALLTYPE BeginOpen( 
        /* [in] */ IFabricStatelessServicePartition *partition,
        /* [in] */ IFabricAsyncOperationCallback *callback,
        /* [retval][out] */ IFabricAsyncOperationContext **context) override;
    
    HRESULT STDMETHODCALLTYPE EndOpen( 
        /* [in] */ IFabricAsyncOperationContext *context,
        /* [retval][out] */ IFabricStringResult **serviceAddress) override;
    
    HRESULT STDMETHODCALLTYPE BeginClose( 
        /* [in] */ IFabricAsyncOperationCallback *callback,
        /* [retval][out] */ IFabricAsyncOperationContext **context) override;
    
    HRESULT STDMETHODCALLTYPE EndClose( 
        /* [in] */ IFabricAsyncOperationContext *context) override;
    
    void STDMETHODCALLTYPE Abort( void) override;

private:
    std::unique_ptr<server> server_;
    boost::asio::io_context io_context_;
    ULONG port_;
    std::wstring hostname_;
    // background threads for server;
    std::vector<std::thread> server_threads_;
};