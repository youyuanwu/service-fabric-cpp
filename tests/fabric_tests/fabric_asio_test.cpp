// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------
#include <boost/ut.hpp>

#include "FabricClient.h"
#include "FabricTypes.h"

#include "servicefabric/asio_callback.hpp"
#include "servicefabric/fabric_client.hpp"
#include "servicefabric/fabric_error.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <latch>

// Tests for reverse wrapper
// impl is coro but the interface is fabric async.
// This is an example of how to impl fabric async interface,
// but the internal is using asio coroutine.
// Similar technique can be used for winrt.

#include "FabricRuntime.h"

#include "servicefabric/string_result.hpp"
#include "servicefabric/waitable_callback.hpp"
#include <winrt/base.h>

#include <any>

namespace sf = servicefabric;
namespace net = boost::asio;

class myctx : public winrt::implements<myctx, IFabricAsyncOperationContext> {
public:
  myctx() {};
  void SetAny(std::any &&a) { a_ = std::move(a); }
  std::any &GetAny() { return a_; }

  BOOLEAN STDMETHODCALLTYPE IsCompleted(void) override { return false; }

  BOOLEAN STDMETHODCALLTYPE CompletedSynchronously(void) override {
    return false;
  }

  HRESULT STDMETHODCALLTYPE get_Callback(
      /* [retval][out] */ IFabricAsyncOperationCallback **callback) override {
    // no callback for now;
    UNREFERENCED_PARAMETER(callback);
    return E_ABORT;
  }

  HRESULT STDMETHODCALLTYPE Cancel(void) override { return S_OK; }

private:
  std::any a_;
};

struct ctxpayload {
  HRESULT hr;
  winrt::com_ptr<IFabricStringResult> addr;
};

class appInstanceImpl {
public:
  appInstanceImpl(net::io_context &io_context) : io_context_(io_context) {}

  // coro backend api impl
  boost::asio::awaitable<HRESULT>
  Open(IFabricStatelessServicePartition *partition,
       IFabricStringResult **serviceAddress) {
    UNREFERENCED_PARAMETER(partition);
    *serviceAddress = winrt::make<sf::string_result>(L"myaddress").detach();
    co_return S_OK;
  }
  boost::asio::awaitable<HRESULT> Close() { co_return S_OK; }

  // callback backend api impl
  // token type void(ec, std::string reply)
  // msg is repeated.
  template <typename Token>
  auto ProcessMessage(std::string data, Token &&token) {
    std::function<void(void)> f = [data, token = std::move(token)]() {
      token({}, std::string(data) + std::string(data));
    };
    return net::post(io_context_, std::move(f));
  }

private:
  net::io_context &io_context_;
};

// wrapped coro api.
class appInstance
    : public winrt::implements<appInstance, IFabricStatelessServiceInstance> {
public:
  appInstance(net::io_context &io_context)
      : io_context_(io_context), impl_(io_context) {}

  HRESULT STDMETHODCALLTYPE BeginOpen(
      /* [in] */ IFabricStatelessServicePartition *partition,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {

    winrt::com_ptr<IFabricAsyncOperationContext> ctx = winrt::make<myctx>();
    winrt::com_ptr<IFabricAsyncOperationCallback> cb;
    cb.copy_from(callback);

    auto f2 =
        [this](winrt::com_ptr<IFabricStatelessServicePartition> fpartition,
               winrt::com_ptr<IFabricAsyncOperationCallback> fcallback,
               winrt::com_ptr<IFabricAsyncOperationContext> fctx)
        -> net::awaitable<void> {
      // add the result info in ctx so that end operation can access it.
      auto cctx = dynamic_cast<myctx *>(fctx.get());
      assert(cctx != nullptr);

      // open server and return address.
      std::shared_ptr<ctxpayload> p = std::make_shared<ctxpayload>();

      // Need a any context
      p->hr = co_await impl_.Open(fpartition.get(), p->addr.put());

      // We always invoke the callback
      // ctx will always have the internacl data set when endopen is invoked
      std::any a = std::move(p);
      cctx->SetAny(std::move(a));
      fcallback->Invoke(cctx);
    };

    winrt::com_ptr<IFabricStatelessServicePartition> fpartition;
    fpartition.copy_from(partition);

    net::co_spawn(io_context_, std::bind(f2, fpartition, cb, ctx),
                  net::detached);

    *context = ctx.detach();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE EndOpen(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricStringResult **serviceAddress) override {
    auto cctx = dynamic_cast<myctx *>(context);
    assert(cctx != nullptr);
    std::any &content = cctx->GetAny();
    std::shared_ptr<ctxpayload> p =
        std::any_cast<std::shared_ptr<ctxpayload>>(content);

    if (p->hr != S_OK) {
      return p->hr;
    }
    assert(p->addr != nullptr);
    *serviceAddress = p->addr.detach();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE BeginClose(
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
    UNREFERENCED_PARAMETER(callback);
    UNREFERENCED_PARAMETER(context);
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE EndClose(
      /* [in] */ IFabricAsyncOperationContext *context) override {
    UNREFERENCED_PARAMETER(context);
    return S_OK;
  }

  void STDMETHODCALLTYPE Abort(void) override {}

private:
  net::io_context &io_context_;
  appInstanceImpl impl_;
};

// example of converting callback style api to begin/end style api.
class appInstance2 {
public:
  appInstance2(net::io_context &io_context)
      : io_context_(io_context), impl_(io_context) {}

  HRESULT BeginProcessMessage(
      std::string data,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) {
    winrt::com_ptr<IFabricAsyncOperationContext> ctx = winrt::make<myctx>();
    winrt::com_ptr<IFabricAsyncOperationCallback> cb;
    cb.copy_from(callback);

    // copy to output
    ctx.get()->AddRef();
    *context = ctx.get();

    impl_.ProcessMessage(
        data, [ctx, cb](boost::system::error_code ec, std::string reply) {
          // TODO : pass on ec. But in this example ec is not used.
          UNREFERENCED_PARAMETER(ec);

          std::any a = std::move(reply);
          auto cctx = dynamic_cast<myctx *>(ctx.get());
          assert(cctx != nullptr);
          cctx->SetAny(std::move(a));
          cb->Invoke(cctx);
        });
    return S_OK;
  }

  HRESULT EndProcessMessage(
      /* [in] */ IFabricAsyncOperationContext *context, std::string *reply) {
    UNREFERENCED_PARAMETER(context);
    auto cctx = dynamic_cast<myctx *>(context);
    assert(cctx != nullptr);
    std::any &content = cctx->GetAny();
    std::string reply_a = std::any_cast<std::string>(content);

    *reply = reply_a;
    return S_OK;
  }

private:
  net::io_context &io_context_;
  appInstanceImpl impl_;
};

boost::ut::suite errors = [] {
  using namespace boost::ut;

  // disabled for requiring a running SF cluster
  if (false) {
    "asio_callback"_test = [] {
      winrt::com_ptr<IFabricQueryClient> client;

      HRESULT hr = ::FabricCreateLocalClient(IID_IFabricQueryClient,
                                             (void **)client.put());

      expect(hr == S_OK >> fatal);

      // try use asio ptr
      boost::system::error_code ec;
      net::io_context io_context;

      auto lamda_callback = [client](IFabricAsyncOperationContext *ctx) {
        winrt::com_ptr<IFabricGetNodeListResult> result;
        HRESULT hr = client->EndGetNodeList(ctx, result.put());

        expect(hr == S_OK >> fatal);

        const FABRIC_NODE_QUERY_RESULT_LIST *nodes = result->get_NodeList();
        expect(nodes != nullptr >> fatal);
      };

      winrt::com_ptr<IFabricAsyncOperationCallback> callback =
          winrt::make<sf::AsioCallback>(lamda_callback,
                                        io_context.get_executor());

      winrt::com_ptr<IFabricAsyncOperationContext> ctx;
      FABRIC_NODE_QUERY_DESCRIPTION node = {};
      hr = client->BeginGetNodeList(&node, 1000, callback.get(), ctx.put());
      expect(hr == S_OK >> fatal);

      io_context.run();
    };

    // disabled
    "asio_waitable_callback"_test = [] {
      winrt::com_ptr<IFabricQueryClient> client;

      HRESULT hr = ::FabricCreateLocalClient(IID_IFabricQueryClient,
                                             (void **)client.put());

      expect(hr == S_OK >> fatal);

      auto f = [&]() -> net::awaitable<void> {
        auto executor = co_await net::this_coro::executor;
        HRESULT hr = S_OK;
        // this is a obj holder
        winrt::com_ptr<sf::IAwaitableCallback> callback =
            winrt::make<sf::AsioAwaitableCallback>(executor);

        winrt::com_ptr<IFabricAsyncOperationContext> ctx;
        FABRIC_NODE_QUERY_DESCRIPTION node = {};
        hr = client->BeginGetNodeList(&node, 1000, callback.get(), ctx.put());

        expect(hr == S_OK >> fatal);
        co_await callback->await();

        winrt::com_ptr<IFabricGetNodeListResult> result;
        hr = client->EndGetNodeList(ctx.get(), result.put());

        expect(hr == S_OK >> fatal);

        const FABRIC_NODE_QUERY_RESULT_LIST *nodes = result->get_NodeList();
        expect(nodes != nullptr >> fatal);
      };
      net::io_context io_context;

      net::co_spawn(io_context, f, net::detached);
      net::co_spawn(io_context, f, net::detached);

      io_context.run();
    };

    // disabled.
    "asio_waitable_fabric_client"_test = [] {
      winrt::com_ptr<IFabricQueryClient> client;

      HRESULT hr = ::FabricCreateLocalClient(IID_IFabricQueryClient,
                                             (void **)client.put());

      expect(hr == S_OK);

      // try use asio ptr
      boost::system::error_code ec;
      net::io_context io_context;

      sf::AwaitableFabricQueryClient fc(client);

      auto f = [&]() -> net::awaitable<void> {
        FABRIC_NODE_QUERY_DESCRIPTION node = {};
        winrt::com_ptr<IFabricGetNodeListResult> result;
        HRESULT lhr = co_await fc.GetNodeListExample(&node, result.put());
        expect(lhr == S_OK >> fatal);
        expect(result->get_NodeList() != nullptr >> fatal);
      };

      net::co_spawn(io_context, f, net::detached);

      auto f2 = [&]() -> net::awaitable<void> {
        HRESULT lhr = S_OK;
        FABRIC_APPLICATION_TYPE_QUERY_DESCRIPTION query = {};
        winrt::com_ptr<IFabricGetApplicationTypeListResult> result;
        lhr = co_await fc.GetApplicationTypeList(&query, result.put());
        expect(lhr == S_OK >> fatal);
        expect(result->get_ApplicationTypeList() != nullptr >> fatal);
      };

      net::co_spawn(io_context, f2, net::detached);

      winrt::com_ptr<IFabricHealthClient> healthClient;

      hr = ::FabricCreateLocalClient(IID_IFabricHealthClient,
                                     (void **)healthClient.put());
      expect(hr == S_OK >> fatal);

      sf::AwaitableFabricHealthClient hc(healthClient);

      auto fhealth = [&]() -> net::awaitable<void> {
        HRESULT lhr = S_OK;
        FABRIC_CLUSTER_HEALTH_POLICY query = {};
        winrt::com_ptr<IFabricClusterHealthResult> result;
        lhr = co_await hc.GetClusterHealth(&query, result.put());
        expect(lhr == S_OK >> fatal);
        expect(result->get_ClusterHealth() != nullptr >> fatal);
      };
      net::co_spawn(io_context, fhealth, net::detached);

      auto fhealth2 = [&]() -> net::awaitable<void> {
        HRESULT lhr = S_OK;
        std::wstring nodeName =
            L"_Node_0"; // This is the name in default cluster
        FABRIC_CLUSTER_HEALTH_POLICY query = {};
        {
          winrt::com_ptr<IFabricNodeHealthResult> result;
          lhr =
              co_await hc.GetNodeHealth(nodeName.c_str(), &query, result.put());
          expect(lhr == S_OK >> fatal);
          expect(result->get_NodeHealth() != nullptr >> fatal);
        }
        // get a unknown node and check error
        {
          winrt::com_ptr<IFabricNodeHealthResult> result;
          lhr = co_await hc.GetNodeHealth(L"BadNodeName", &query, result.put());
          expect(lhr == FABRIC_E_HEALTH_ENTITY_NOT_FOUND)
              << "not found: " + sf::get_fabric_error_str(lhr);
          expect(lhr == FABRIC_E_HEALTH_ENTITY_NOT_FOUND >> fatal);
          expect(result.get() == nullptr >> fatal);
        }
      };
      net::co_spawn(io_context, fhealth2, net::detached);

      io_context.run();
    };
  } // end if false
  "asio_fabric_reverse"_test = [] {
    net::io_context ioc;

    winrt::com_ptr<IFabricStatelessServiceInstance> svc =
        winrt::make<appInstance>(ioc);

    // ioc must run after the job is posted to it,
    // otherwise ioc will see no jobs and finish run immediately.
    std::latch lch(1);

    // use waitable ctx
    auto f = [&]() {
      // test coro backend api.
      winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
          winrt::make<sf::FabricAsyncOperationWaitableCallback>();

      winrt::com_ptr<IFabricAsyncOperationContext> ctx;

      HRESULT hr = svc->BeginOpen(nullptr, // partition
                                  callback.get(), ctx.put());
      // job is pushed to ioc in svc
      lch.count_down();
      expect(hr == S_OK >> fatal);
      callback->Wait();
      winrt::com_ptr<IFabricStringResult> addr;
      hr = svc->EndOpen(ctx.get(), addr.put());
      expect(hr == S_OK >> fatal);
      bool str_eq =
          (std::wstring(addr->get_String()) == std::wstring(L"myaddress"));
      expect(str_eq >> fatal);
    };

    std::latch lch2(1);
    auto f2 = [&]() {
      // test callback backend api
      appInstance2 app2(ioc);
      winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
          winrt::make<sf::FabricAsyncOperationWaitableCallback>();
      winrt::com_ptr<IFabricAsyncOperationContext> ctx;

      HRESULT hr = app2.BeginProcessMessage("hello", callback.get(), ctx.put());
      lch2.count_down();
      expect(hr == S_OK >> fatal);
      callback->Wait();
      std::string reply;
      hr = app2.EndProcessMessage(ctx.get(), &reply);
      expect(hr == S_OK >> fatal);
      expect(std::string("hellohello") == reply >> fatal);
    };

    // start the operation
    std::jthread th(f);
    std::jthread th2(f2);

    std::jthread th_io([&]() {
      lch.wait();
      lch2.wait();
      ioc.run();
    });

    th.join();
    th2.join();
    th_io.join();
  };
};

int main() {}
