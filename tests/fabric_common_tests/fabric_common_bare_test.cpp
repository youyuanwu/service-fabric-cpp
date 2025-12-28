#include <boost/ut.hpp>

#include "AsyncOperation.h"
#include "ComAsyncOperationContext.h"
#include "ComComponentRoot.h"
#include "ComProxyAsyncOperation.h"
#include "CompletedAsyncOperation.h"
#include "servicefabric/waitable_callback.hpp"
#include <iostream>
#include <winrt/base.h>

#include <latch>
#include <random>

// test async operations without using components.
using namespace Common;

class TestLogger {
public:
  void Write(std::string msg) {
    std::hash<std::thread::id> hash{};
    std::size_t id = hash(std::this_thread::get_id());
    std::lock_guard<std::mutex> lk(mtx_);
    DBG_UNREFERENCED_LOCAL_VARIABLE(id);
    // uncomment for debugging.
    std::cout << "[" << id << "] " << msg << std::endl;
  }

private:
  std::mutex mtx_;
};

static std::unique_ptr<TestLogger> GLogger = std::make_unique<TestLogger>();

class ComponentB {
  DENY_COPY(ComponentB)
public:
  ComponentB() {}

  AsyncOperationSPtr BeginFooB1(const AsyncCallback callback,
                                AsyncOperationSPtr const &parent) {
    GLogger->Write("BeginFooB1");
    return AsyncOperation::CreateAndStart<CompletedAsyncOperation>(callback,
                                                                   parent);
  }
  ErrorCode EndFooB1(AsyncOperationSPtr const &fooB1Operation) {
    GLogger->Write("EndFooB1");
    return CompletedAsyncOperation::End(fooB1Operation);
  }
};

class ComponentA {
  DENY_COPY(ComponentA)

public:
  ComponentA() { componentB = std::make_unique<ComponentB>(); }

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
    TryComplete(fooB1Operation->Parent, error);
  }
};

AsyncOperationSPtr ComponentA::BeginFooA(const AsyncCallback callback,
                                         AsyncOperationSPtr const &parent) {
  GLogger->Write("BeginFooA");
  return AsyncOperation::CreateAndStart<FooAAsyncOperation>(*this, callback,
                                                            parent);
}

ErrorCode ComponentA::EndFooA(AsyncOperationSPtr const &fooAOperation) {
  GLogger->Write("EndFooA");
  return FooAAsyncOperation::End(fooAOperation);
}

boost::ut::suite errors = [] {
  using namespace boost::ut;

  "basic"_test = [] {
    auto componentA = std::make_shared<ComponentA>();
    auto componentAPtr = componentA.get();
    std::latch sync(1);

    auto operation = componentA->BeginFooA(
        [componentAPtr, &sync](AsyncOperationSPtr const &fooAOperation) {
          GLogger->Write("BeginFooA callback");
          if (!fooAOperation->CompletedSynchronously) {
            GLogger->Write("BeginFooA callback async invoke");
            ErrorCode errorCode = componentAPtr->EndFooA(fooAOperation);
            expect(errorCode.IsSuccess());
            sync.count_down();
          }
        },
        AsyncOperationRoot<bool>::Create(true)); // use a dummy root op
    GLogger->Write("ComponentA op started");
    if (operation->CompletedSynchronously) {
      GLogger->Write("BeginFooA Completed same thread.");
      ErrorCode errorCode = componentAPtr->EndFooA(operation);
      expect(errorCode.IsSuccess());
      sync.count_down();
    }
    sync.wait();
  };
};

int main() {}