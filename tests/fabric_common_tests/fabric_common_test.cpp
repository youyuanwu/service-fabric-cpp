#define BOOST_TEST_MODULE fabric_asio_tests

#include "AsyncOperation.h"
#include "ComAsyncOperationContext.h"
#include "ComComponentRoot.h"
#include "ComProxyAsyncOperation.h"
#include "CompletedAsyncOperation.h"
#include "ComponentRoot.h"
#include "RootedObject.h"
#include "RootedObjectPointer.h"
#include "servicefabric/waitable_callback.hpp"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <winrt/base.h>
// #include <future>
#include <latch>
#include <random>

class TestLogger {
public:
  void Write(std::string msg) {
    std::hash<std::thread::id> hash{};
    std::size_t id = hash(std::this_thread::get_id());
    std::lock_guard<std::mutex> lk(mtx_);
    std::cout << "[" << id << "] " << msg << std::endl;
  }

private:
  std::mutex mtx_;
};

static std::unique_ptr<TestLogger> GLogger = std::make_unique<TestLogger>();

// class exampleOp : public Common::AsyncOperation {
// public:
//   void OnStart(Common::AsyncOperationSPtr const &thisSPtr) override {
//     GLogger->Write("OnStart called");
//   }
// };

using namespace Common;

class ComponentC : RootedObject {
  DENY_COPY(ComponentC)

public:
  ComponentC(ComponentRoot const &root) : RootedObject(root) {}

  AsyncOperationSPtr BeginFooC11(const AsyncCallback callback,
                                 AsyncOperationSPtr const &parent) {
    GLogger->Write("BeginFooC11");
    return AsyncOperation::CreateAndStart<CompletedAsyncOperation>(callback,
                                                                   parent);
  }

  ErrorCode EndFooC11(AsyncOperationSPtr const &fooC11Operation) {
    GLogger->Write("EndFooC11");
    return CompletedAsyncOperation::End(fooC11Operation);
  }

  AsyncOperationSPtr BeginFooC12(const AsyncCallback callback,
                                 AsyncOperationSPtr const &parent) {
    GLogger->Write("BeginFooC12");
    return AsyncOperation::CreateAndStart<CompletedAsyncOperation>(callback,
                                                                   parent);
  }
  ErrorCode EndFooC12(AsyncOperationSPtr const &fooC12Operation) {
    GLogger->Write("EndFooC12");
    return CompletedAsyncOperation::End(fooC12Operation);
  }

  AsyncOperationSPtr BeginFooC21(const AsyncCallback callback,
                                 AsyncOperationSPtr const &parent) {
    GLogger->Write("BeginFooC21");
    return AsyncOperation::CreateAndStart<CompletedAsyncOperation>(callback,
                                                                   parent);
  }
  ErrorCode EndFooC21(AsyncOperationSPtr const &fooC21Operation) {
    GLogger->Write("EndFooC21");
    return CompletedAsyncOperation::End(fooC21Operation);
  }

  AsyncOperationSPtr BeginFooC22(const AsyncCallback callback,
                                 AsyncOperationSPtr const &parent) {
    GLogger->Write("BeginFooC22");
    return AsyncOperation::CreateAndStart<CompletedAsyncOperation>(callback,
                                                                   parent);
  }
  ErrorCode EndFooC22(AsyncOperationSPtr const &fooC22Operation) {
    GLogger->Write("EndFooC22");
    return CompletedAsyncOperation::End(fooC22Operation);
  }
};

class FooB1AsyncOperation;

class ComponentB : RootedObject {
  DENY_COPY(ComponentB)

public:
  ComponentB(ComponentRoot const &root)
      : RootedObject(root), componentC(std::make_unique<ComponentC>(root)) {}

  AsyncOperationSPtr BeginFooB1(const AsyncCallback callback,
                                AsyncOperationSPtr const &parent);
  ErrorCode EndFooB1(AsyncOperationSPtr const &fooB1Operation);

  AsyncOperationSPtr BeginFooB2(const AsyncCallback callback,
                                AsyncOperationSPtr const &parent);
  ErrorCode EndFooB2(AsyncOperationSPtr const &fooB2Operation);

private:
  std::unique_ptr<ComponentC> componentC;
  class FooB1AsyncOperation;
  class FooB2AsyncOperation;
};

class ComponentB::FooB1AsyncOperation : public AsyncOperation {
  DENY_COPY(FooB1AsyncOperation)

public:
  FooB1AsyncOperation(ComponentB &owner, const AsyncCallback callback,
                      AsyncOperationSPtr const &parent)
      : owner_(owner), AsyncOperation(callback, parent) {
    // Trace.WriteInfo(TraceType, "FooB1AsyncOperation()");
    GLogger->Write("FooB1AsyncOperation()");
  }

  ~FooB1AsyncOperation() {
    // Trace.WriteInfo(TraceType, "~FooB1AsyncOperation()");
    GLogger->Write("~FooB1AsyncOperation()");
  }

  static ErrorCode End(AsyncOperationSPtr const &fooB1Operation) {
    auto thisPtr = AsyncOperation::End<FooB1AsyncOperation>(fooB1Operation);
    return thisPtr->Error;
  }

protected:
  void OnStart(AsyncOperationSPtr const &thisSPtr) {
    // if (Common::CancelScenario)
    if (false) {
      Cancel();
    } else {
      auto operation = owner_.componentC->BeginFooC11(
          [this](AsyncOperationSPtr const &fooC11Operation) {
            this->OnFooC11Completed(fooC11Operation);
          },
          thisSPtr);
      if (operation->CompletedSynchronously) {
        FinishFooC11(operation);
      }
    }
  }

  void OnCancel() {
    // Trace.WriteInfo(TraceType, "FooB1AsyncOperation::OnCancel()");
    GLogger->Write("FooB1AsyncOperation::OnCancel()");
    TryComplete(shared_from_this(), ErrorCodeValue::OperationCanceled);
  }

private:
  ComponentB &owner_;

  void OnFooC11Completed(AsyncOperationSPtr const &fooC11Operation) {
    if (!fooC11Operation->CompletedSynchronously) {
      FinishFooC11(fooC11Operation);
    }
  }

  void FinishFooC11(AsyncOperationSPtr const &fooC11Operation) {
    ErrorCode error = owner_.componentC->EndFooC11(fooC11Operation);
    if (!error.IsSuccess()) {
      TryComplete(fooC11Operation->Parent, error);
    } else {
      auto operation = owner_.componentC->BeginFooC12(
          [this](AsyncOperationSPtr const &fooC12Operation) {
            this->OnFooC12Completed(fooC12Operation);
          },
          fooC11Operation->Parent);
      if (operation->CompletedSynchronously) {
        FinishFooC12(operation);
      }
    }
  }

  void OnFooC12Completed(AsyncOperationSPtr const &fooC12Operation) {
    if (!fooC12Operation->CompletedSynchronously) {
      FinishFooC12(fooC12Operation);
    }
  }

  void FinishFooC12(AsyncOperationSPtr const &fooC12Operation) {
    TryComplete(fooC12Operation->Parent,
                owner_.componentC->EndFooC12(fooC12Operation));
  }
};
class ComponentB::FooB2AsyncOperation : public AsyncOperation {
  DENY_COPY(FooB2AsyncOperation)

public:
  FooB2AsyncOperation(ComponentB &owner, const AsyncCallback callback,
                      AsyncOperationSPtr const &parent)
      : owner_(owner), AsyncOperation(callback, parent) {
    // Trace.WriteInfo(TraceType, "FooB2AsyncOperation()");
    GLogger->Write("FooB2AsyncOperation()");
  }

  ~FooB2AsyncOperation() {
    // Trace.WriteInfo(TraceType, "~FooB2AsyncOperation()");
    GLogger->Write("~FooB2AsyncOperation()");
  }

  static ErrorCode End(AsyncOperationSPtr const &fooB2Operation) {
    auto thisPtr = AsyncOperation::End<FooB2AsyncOperation>(fooB2Operation);
    return thisPtr->Error;
  }

protected:
  void OnStart(AsyncOperationSPtr const &thisSPtr) {
    // flip randomly to use sync or async.
    static thread_local std::mt19937 generator(clock());
    std::uniform_int_distribution<int> distribution(0, 10);
    bool async = distribution(generator) % 2;
    if (async) {
      GLogger->Write("FooB2AsyncOperation::OnStart() Launch thread");
      // launch this in another thread
      AsyncOperationSPtr copy = thisSPtr;
      std::thread([copy]() {
        copy->Get<FooB2AsyncOperation>(copy)->onStartInternal(copy);
      }).detach();
    } else {
      GLogger->Write("FooB2AsyncOperation::OnStart() sync");
      this->onStartInternal(thisSPtr);
    }
  }

  void onStartInternal(AsyncOperationSPtr const &thisSPtr) {
    // if (Common::CancelScenario)
    if (false) {
      Cancel();
    } else {
      auto operation = owner_.componentC->BeginFooC21(
          [this](AsyncOperationSPtr const &fooC11Operation) {
            this->OnFooC21Completed(fooC11Operation);
          },
          thisSPtr);
      if (operation->CompletedSynchronously) {
        FinishFooC21(operation);
      }
    }
  }

  void OnCancel() {
    // Trace.WriteInfo(TraceType, "FooB2AsyncOperation::OnCancel()");
    GLogger->Write("FooB2AsyncOperation::OnCancel()");
    TryComplete(shared_from_this(), ErrorCodeValue::OperationCanceled);
  }

private:
  ComponentB &owner_;

  void OnFooC21Completed(AsyncOperationSPtr const &fooC11Operation) {
    if (!fooC11Operation->CompletedSynchronously) {
      FinishFooC21(fooC11Operation);
    }
  }

  void FinishFooC21(AsyncOperationSPtr const &fooC11Operation) {
    ErrorCode error = owner_.componentC->EndFooC21(fooC11Operation);
    if (!error.IsSuccess()) {
      TryComplete(fooC11Operation->Parent, error);
    } else {
      auto operation = owner_.componentC->BeginFooC22(
          [this](AsyncOperationSPtr const &fooC12Operation) {
            this->OnFooC22Completed(fooC12Operation);
          },
          fooC11Operation->Parent);
      if (operation->CompletedSynchronously) {
        FinishFooC22(operation);
      }
    }
  }

  void OnFooC22Completed(AsyncOperationSPtr const &fooC12Operation) {
    if (!fooC12Operation->CompletedSynchronously) {
      FinishFooC22(fooC12Operation);
    }
  }

  void FinishFooC22(AsyncOperationSPtr const &fooC12Operation) {
    TryComplete(fooC12Operation->Parent,
                owner_.componentC->EndFooC22(fooC12Operation));
  }
};

AsyncOperationSPtr ComponentB::BeginFooB1(const AsyncCallback callback,
                                          AsyncOperationSPtr const &parent) {
  // Trace.WriteInfo(TraceType, "BeginFooB1");
  GLogger->Write("BeginFooB1");
  return AsyncOperation::CreateAndStart<FooB1AsyncOperation>(*this, callback,
                                                             parent);
}

ErrorCode ComponentB::EndFooB1(AsyncOperationSPtr const &fooB1Operation) {
  // Trace.WriteInfo(TraceType, "EndFooB1");
  GLogger->Write("EndFooB1");
  return FooB1AsyncOperation::End(fooB1Operation);
}

AsyncOperationSPtr ComponentB::BeginFooB2(const AsyncCallback callback,
                                          AsyncOperationSPtr const &parent) {
  // Trace.WriteInfo(TraceType, "BeginFooB2");
  GLogger->Write("BeginFooB2");
  return AsyncOperation::CreateAndStart<FooB2AsyncOperation>(*this, callback,
                                                             parent);
}

ErrorCode ComponentB::EndFooB2(AsyncOperationSPtr const &fooB2Operation) {
  // Trace.WriteInfo(TraceType, "EndFooB2");
  GLogger->Write("EndFooB2");
  return FooB2AsyncOperation::End(fooB2Operation);
}

// TODO: define a abstractA class so that comproxy and componentA share?

class ComponentA : public ComponentRoot {
  DENY_COPY(ComponentA)

public:
  ComponentA() : ComponentRoot() {
    componentB = std::make_unique<ComponentB>(*this);
  }

  AsyncOperationSPtr BeginFooA(const AsyncCallback callback,
                               AsyncOperationSPtr const &parent);
  ErrorCode EndFooA(AsyncOperationSPtr const &fooAOperation);

private:
  std::unique_ptr<ComponentB> componentB;
  class FooAAsyncOperation;
};

class ComponentA::FooAAsyncOperation : public AsyncOperation {
  DENY_COPY(FooAAsyncOperation)

public:
  FooAAsyncOperation(ComponentA &owner, const AsyncCallback callback,
                     AsyncOperationSPtr const &parent)
      : owner_(owner), AsyncOperation(callback, parent) {
    // Trace.WriteInfo(TraceType, "FooAAsyncOperation");
    GLogger->Write("FooAAsyncOperation");
  }

  ~FooAAsyncOperation() {
    // Trace.WriteInfo(TraceType, "~FooAAsyncOperation");
    GLogger->Write("~FooAAsyncOperation");
  }

  static ErrorCode End(AsyncOperationSPtr const &fooAAOperation) {
    auto thisPtr = AsyncOperation::End<FooAAsyncOperation>(fooAAOperation);
    return thisPtr->Error;
  }

protected:
  void OnStart(AsyncOperationSPtr const &thisSPtr) {
    auto operation = owner_.componentB->BeginFooB1(
        [this](AsyncOperationSPtr const &fooB1Operation) {
          this->OnFooB1Completed(fooB1Operation);
        },
        thisSPtr);
    if (operation->CompletedSynchronously) {
      FinishFooB1(operation);
    }
  }

private:
  ComponentA &owner_;

  void OnFooB1Completed(AsyncOperationSPtr const &fooB1Operation) {
    if (!fooB1Operation->CompletedSynchronously) {
      FinishFooB1(fooB1Operation);
    }
  }

  void FinishFooB1(AsyncOperationSPtr const &fooB1Operation) {
    ErrorCode error = owner_.componentB->EndFooB1(fooB1Operation);
    if (!error.IsSuccess()) {
      TryComplete(fooB1Operation->Parent, error);
    } else {
      auto operation = owner_.componentB->BeginFooB2(
          [this](AsyncOperationSPtr const &fooB2Operation) {
            this->OnFooB2Completed(fooB2Operation);
          },
          fooB1Operation->Parent);
      if (operation->CompletedSynchronously) {
        FinishFooB2(operation);
      }
    }
  }

  void OnFooB2Completed(AsyncOperationSPtr const &fooB2Operation) {
    if (!fooB2Operation->CompletedSynchronously) {
      FinishFooB2(fooB2Operation);
    }
  }

  void FinishFooB2(AsyncOperationSPtr const &fooB2Operation) {
    ErrorCode error = owner_.componentB->EndFooB2(fooB2Operation);
    TryComplete(fooB2Operation->Parent, error);
  }
};

AsyncOperationSPtr ComponentA::BeginFooA(const AsyncCallback callback,
                                         AsyncOperationSPtr const &parent) {
  // Trace.WriteInfo(TraceType, "BeginFooA");
  GLogger->Write("BeginFooA");
  return AsyncOperation::CreateAndStart<FooAAsyncOperation>(*this, callback,
                                                            parent);
}

ErrorCode ComponentA::EndFooA(AsyncOperationSPtr const &fooAOperation) {
  // Trace.WriteInfo(TraceType, "EndFooA");
  GLogger->Write("EndFooA");
  return FooAAsyncOperation::End(fooAOperation);
}

static std::atomic_bool test_bool = false;

// not used for now.
struct BasicScenarioTestHelper {
  static void OnFooACompleted(ComponentA *const componentAPtr,
                              AsyncOperationSPtr const &fooAOperation) {
    // Trace.WriteInfo(TraceType, "OnFooACompleted callback called");
    GLogger->Write("OnFooACompleted callback called");
    if (!fooAOperation->CompletedSynchronously) {
      FinishFooA(componentAPtr, fooAOperation);
    }
  }

  static void FinishFooA(ComponentA *const componentAPtr,
                         AsyncOperationSPtr const &fooAOperation) {
    ErrorCode errorCode = componentAPtr->EndFooA(fooAOperation);
    // if (CancelScenario == false)
    if (true) {
      BOOST_REQUIRE(errorCode.IsSuccess());
    } else {
      BOOST_REQUIRE(errorCode.ReadValue() == ErrorCodeValue::OperationCanceled);
      // Trace.WriteInfo(TraceType, "Operation canceled successfully");
    }

    test_bool = true;
  }
};

// convert native to com.
// impl of COM wrappers
static const GUID MYID = {0xd80af29b,
                          0xfb0,
                          0x4c68,
                          {0x9e, 0xfb, 0xb5, 0x41, 0x58, 0x24, 0x44, 0xf6}};

class ComFooAsyncOperation : public Common::ComAsyncOperationContext {
  COM_INTERFACE_AND_DELEGATE_LIST(ComFooAsyncOperation, MYID,
                                  ComFooAsyncOperation,
                                  ComAsyncOperationContext)
public:
  ComFooAsyncOperation(__in Common::RootedObjectPointer<ComponentA> &impl)
      : impl_(impl) {}

  void OnStart(AsyncOperationSPtr const &proxySPtr) override {
    auto operation = impl_->BeginFooA(
        [this](AsyncOperationSPtr const &operation) {
          this->OnFinish(operation, false);
        },
        proxySPtr);

    this->OnFinish(operation, true);
  }

  void OnFinish(AsyncOperationSPtr const &operation,
                bool expectedCompletedSynchronously) {
    if (expectedCompletedSynchronously != operation->CompletedSynchronously) {
      return;
    }
    auto error = impl_->EndFooA(operation);
    TryComplete(operation->Parent, error.ToHResult());
  }

  HRESULT Initialize(__in IFabricAsyncOperationCallback *callback,
                     Common::ComponentRootSPtr const &rootSPtr) {
    HRESULT hr = this->ComAsyncOperationContext::Initialize(rootSPtr, callback);
    if (FAILED(hr)) {
      return hr;
    }
    return S_OK;
  }

  static HRESULT End(__in IFabricAsyncOperationContext *context) {
    if (context == NULL) {
      return E_POINTER;
    }

    ComPointer<ComFooAsyncOperation> thisOperation(context, MYID);

    // superclass end
    HRESULT hr = thisOperation->ComAsyncOperationContextEnd();
    return hr;
  }

  HRESULT ComAsyncOperationContextEnd() {
    return ComAsyncOperationContext::End();
  }

private:
  Common::RootedObjectPointer<ComponentA> impl_;
};

MIDL_INTERFACE("e30cb081-548f-44e7-9244-ab788103d12c")
IAComponent : public IUnknown {
public:
  virtual HRESULT STDMETHODCALLTYPE BeginFooA(
      /* [in] */ IFabricAsyncOperationCallback * callback,
      /* [out, retval] */ IFabricAsyncOperationContext * *context) = 0;

  virtual HRESULT STDMETHODCALLTYPE EndFooA(
      /* [in] */ IFabricAsyncOperationContext * context) = 0;
};

class ComAComponent : public winrt::implements<ComAComponent, IAComponent> {
public:
  ComAComponent(Common::RootedObjectPointer<ComponentA> const &impl)
      : impl_(impl) {}

  HRESULT BeginFooA(
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [out, retval] */ IFabricAsyncOperationContext **context) override {

    ComPointer<IUnknown> rootCPtr;
    HRESULT hr = this->QueryInterface(
        IID_IUnknown, (void **)rootCPtr.InitializationAddress());
    if (FAILED(hr)) {
      return hr;
    }

    auto rootSPtr =
        std::make_shared<ComComponentRoot<IUnknown>>(std::move(rootCPtr));

    auto copyImpl =
        RootedObjectPointer<ComponentA>(impl_.get(), impl_.get_Root());

    ComPointer<ComFooAsyncOperation> operation =
        make_com<ComFooAsyncOperation>(copyImpl);
    hr = operation->Initialize(callback, rootSPtr);

    if (FAILED(hr)) {
      return ComUtility::OnPublicApiReturn(hr);
    }
    return ComUtility::OnPublicApiReturn(
        ComAsyncOperationContext::StartAndDetach(std::move(operation),
                                                 context));
  }

  HRESULT EndFooA(/* [in] */ IFabricAsyncOperationContext *context) override {
    return ComUtility::OnPublicApiReturn(ComFooAsyncOperation::End(context));
  }

private:
  Common::RootedObjectPointer<ComponentA> const &impl_;
};

// convert com to native

class ComProxyComponentA : public Common::ComponentRoot {
  DENY_COPY(ComProxyComponentA);

public:
  ComProxyComponentA(Common::ComPointer<IAComponent> const &comImpl)
      : comImpl_(comImpl) {}

  Common::AsyncOperationSPtr
  BeginFooA(Common::AsyncCallback const &callback,
            Common::AsyncOperationSPtr const &parent);

  Common::ErrorCode EndFooA(Common::AsyncOperationSPtr const &operation);

private:
  class ComProxyComponentAAsyncOperation;
  Common::ComPointer<IAComponent> comImpl_;
};

class ComProxyComponentA::ComProxyComponentAAsyncOperation
    : public Common::ComProxyAsyncOperation {
public:
  ComProxyComponentAAsyncOperation(__in IAComponent &comImpl,
                                   AsyncCallback const &callback,
                                   AsyncOperationSPtr const &parent)
      : ComProxyAsyncOperation(callback, parent), comImpl_(comImpl) {}

  static ErrorCode End(AsyncOperationSPtr const &operation) {
    auto thisPtr =
        AsyncOperation::End<ComProxyComponentAAsyncOperation>(operation);
    return thisPtr->Error;
  }

protected:
  HRESULT
  BeginComAsyncOperation(IFabricAsyncOperationCallback *callback,
                         IFabricAsyncOperationContext **context) override {
    return comImpl_.BeginFooA(callback, context);
  }

  HRESULT EndComAsyncOperation(IFabricAsyncOperationContext *context) override {
    return comImpl_.EndFooA(context);
  }

private:
  IAComponent &comImpl_;
};

Common::AsyncOperationSPtr
ComProxyComponentA::BeginFooA(Common::AsyncCallback const &callback,
                              Common::AsyncOperationSPtr const &parent) {
  auto operation =
      AsyncOperation::CreateAndStart<ComProxyComponentAAsyncOperation>(
          *comImpl_.GetRawPointer(), callback, parent);
  return operation;
}

Common::ErrorCode
ComProxyComponentA::EndFooA(Common::AsyncOperationSPtr const &operation) {
  return ComProxyComponentAAsyncOperation::End(operation);
}

BOOST_AUTO_TEST_SUITE(test_fabric_common)

BOOST_AUTO_TEST_CASE(basic_test) {
  auto componentA = std::make_shared<ComponentA>();
  auto componentAPtr = componentA.get();

  std::latch sync(1);

  auto operation = componentA->BeginFooA(
      [componentAPtr, &sync](AsyncOperationSPtr const &fooAOperation) {
        GLogger->Write("BeginFooA callback async invoke");
        if (!fooAOperation->CompletedSynchronously) {
          ErrorCode errorCode = componentAPtr->EndFooA(fooAOperation);
          BOOST_REQUIRE(errorCode.IsSuccess());
          sync.count_down();
        }
      },
      componentA->CreateAsyncOperationRoot());
  GLogger->Write("ComponentA op started");
  if (operation->CompletedSynchronously) {
    GLogger->Write("BeginFooA callback sync invoke");
    ErrorCode errorCode = componentAPtr->EndFooA(operation);
    BOOST_REQUIRE(errorCode.IsSuccess());
    sync.count_down();
  }
  sync.wait();
}

BOOST_AUTO_TEST_CASE(basic_com_test) {
  // make componentA
  std::shared_ptr<ComponentA> componentA = std::make_shared<ComponentA>();
  Common::RootedObjectPointer<ComponentA> impl(
      componentA.get(), componentA->CreateComponentRoot());

  // make com wrapper
  winrt::com_ptr<IAComponent> ccA = winrt::make<ComAComponent>(impl);

  belt::com::com_ptr<servicefabric::IFabricAsyncOperationWaitableCallback>
      callback =
          servicefabric::FabricAsyncOperationWaitableCallback::create_instance()
              .to_ptr();
  winrt::com_ptr<IFabricAsyncOperationContext> ctx;

  HRESULT hr = ccA->BeginFooA(callback.get(), ctx.put());
  BOOST_REQUIRE_EQUAL(hr, S_OK);
  callback->Wait();
  hr = ccA->EndFooA(ctx.get());
  BOOST_REQUIRE_EQUAL(hr, S_OK);
}

BOOST_AUTO_TEST_CASE(basic_com_proxy_test) {
  // create com component
  // make componentA
  std::shared_ptr<ComponentA> componentA = std::make_shared<ComponentA>();
  Common::RootedObjectPointer<ComponentA> impl(
      componentA.get(), componentA->CreateComponentRoot());
  // make com wrapper
  winrt::com_ptr<IAComponent> ccA = winrt::make<ComAComponent>(impl);
  Common::ComPointer<IAComponent> ccACopy;
  ccACopy.SetAndAddRef(ccA.get());
  // create proxy
  auto proxy = std::make_shared<ComProxyComponentA>(ccACopy);
  auto proxyPtr = proxy.get();
  // test the proxy
  std::latch sync(1);

  auto operation = proxy->BeginFooA(
      [proxyPtr, &sync](AsyncOperationSPtr const &fooAOperation) {
        GLogger->Write("BeginFooA callback async invoke");
        if (!fooAOperation->CompletedSynchronously) {
          ErrorCode errorCode = proxyPtr->EndFooA(fooAOperation);
          BOOST_REQUIRE(errorCode.IsSuccess());
          sync.count_down();
        }
      },
      componentA->CreateAsyncOperationRoot());
  GLogger->Write("ComponentA op started");
  if (operation->CompletedSynchronously) {
    GLogger->Write("BeginFooA callback sync invoke");
    ErrorCode errorCode = proxyPtr->EndFooA(operation);
    BOOST_REQUIRE(errorCode.IsSuccess());
    sync.count_down();
  }
  sync.wait();
}

BOOST_AUTO_TEST_SUITE_END()