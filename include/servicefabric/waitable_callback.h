#pragma once
#include <boost/log/trivial.hpp>
#include "FabricCommon.h"

#include <mutex>
#include <condition_variable>

#include <moderncom/interfaces.h>

namespace servicefabric{

MIDL_INTERFACE("b8fa8b9b-c874-407a-99fe-f3b0aa0ba5e7")
IFabricAsyncOperationWaitableCallback : public IFabricAsyncOperationCallback
{
public:
    // wait for operation to be called
    virtual void STDMETHODCALLTYPE Wait() = 0;
    
};


class FabricAsyncOperationWaitableCallback :
  public belt::com::object<
    FabricAsyncOperationWaitableCallback, // our class
    IFabricAsyncOperationWaitableCallback> // we implement
{

public:
    FabricAsyncOperationWaitableCallback(): m_(), cv_(), ready_(false){}
    void Invoke( /* [in] */ IFabricAsyncOperationContext *context) override{
        {
            std::lock_guard lk(m_);
            ready_ = true;
            BOOST_LOG_TRIVIAL(debug) << "FabricAsyncOperationWaitableCallback::Invoke";
        }
        cv_.notify_all();
    }

    // wait for cv notify
    void Wait(){
        BOOST_LOG_TRIVIAL(debug) << "FabricAsyncOperationWaitableCallback::Wait";
        std::unique_lock lk(m_);
        cv_.wait(lk, [this]{return ready_;});
    }

private:
    std::mutex m_;
    std::condition_variable cv_;
    bool ready_;
};

} // namespace servicefabric