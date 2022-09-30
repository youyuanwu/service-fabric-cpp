#include <boost/log/trivial.hpp>
#include "FabricClient.h"

#include <string>
#include <iostream>

#include <servicefabric/waitable_callback.h>

#include <moderncom/interfaces.h>

#include <thread>

namespace sf = servicefabric;

int main(){
    
    belt::com::com_ptr<IFabricQueryClient> client;

    std::wstring conn = L"hello";
    HRESULT hr = ::FabricCreateLocalClient(IID_IFabricQueryClient, (void **)client.put());
    
    if(hr != NO_ERROR){
        std::cout << "client creation failed" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "FabricCreateLocalClient success" << std::endl;

    //belt::com::com_ptr<IFabricAsyncOperationCallback> callback = MyCallback::create_instance(client).to_ptr();
    
    belt::com::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback = sf::FabricAsyncOperationWaitableCallback::create_instance().to_ptr();

    belt::com::com_ptr<IFabricAsyncOperationContext> ctx;

    FABRIC_NODE_QUERY_DESCRIPTION node = {};
    hr = client->BeginGetNodeList(&node, 1000, callback.get(), ctx.put());
    if(hr != NO_ERROR){
        std::cout << "BeginGetNodeList failed" << std::endl;
        return EXIT_FAILURE;
    }
    callback->Wait();

    belt::com::com_ptr<IFabricGetNodeListResult> result;
    hr = client->EndGetNodeList(ctx.get(), result.put());
    if(hr != NO_ERROR){
        std::cout << "EndGetNodeList failed: "<< hr << std::endl;
        return EXIT_FAILURE;
    }
    const FABRIC_NODE_QUERY_RESULT_LIST * nodes = result->get_NodeList();
    if(nodes != nullptr){
        std::cout << "node count :" << nodes->Count << std::endl;
    }
}