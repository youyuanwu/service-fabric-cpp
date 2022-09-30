#include <boost/log/trivial.hpp>
#include "async_context.hpp"

async_context::async_context(IFabricAsyncOperationCallback *callback) : callback_(callback){
    BOOST_LOG_TRIVIAL(debug) << "async_context::async_context";    
    // do not store callback works

    // invoke callback
    callback_->Invoke(this);
}

BOOLEAN STDMETHODCALLTYPE async_context::IsCompleted(){
    BOOST_LOG_TRIVIAL(debug) << "async_context::IsCompleted";
    return true;
}
BOOLEAN STDMETHODCALLTYPE async_context::CompletedSynchronously(){
    BOOST_LOG_TRIVIAL(debug) << "async_context::CompletedSynchronously";
    return true;
}
HRESULT STDMETHODCALLTYPE async_context::get_Callback( 
    /* [retval][out] */ IFabricAsyncOperationCallback **callback){

    BOOST_LOG_TRIVIAL(debug) << "async_context::get_Callback";

    // callback is return as a reference.
    *callback = callback_.get();
    return S_OK;
}
HRESULT STDMETHODCALLTYPE async_context::Cancel(){
    BOOST_LOG_TRIVIAL(debug) << "async_context::Cancel";
    return S_OK;
}