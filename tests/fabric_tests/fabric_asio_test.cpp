#define BOOST_TEST_MODULE fabric_asio_tests

#include <boost/test/unit_test.hpp>

#include "FabricClient.h"
#include "FabricTypes.h"

#include "servicefabric/asio_callback.hpp"
#include "servicefabric/fabric_client.hpp"
#include "servicefabric/fabric_error.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

namespace sf = servicefabric;
namespace net = boost::asio;

BOOST_AUTO_TEST_SUITE(test_fabric_asio)

BOOST_AUTO_TEST_CASE(test_asio_callback) {
  belt::com::com_ptr<IFabricQueryClient> client;

  HRESULT hr =
      ::FabricCreateLocalClient(IID_IFabricQueryClient, (void **)client.put());

  BOOST_REQUIRE_EQUAL(hr, S_OK);

  // try use asio ptr
  boost::system::error_code ec;
  net::io_context io_context;

  auto lamda_callback = [client](IFabricAsyncOperationContext *ctx) {
    belt::com::com_ptr<IFabricGetNodeListResult> result;
    HRESULT hr = client->EndGetNodeList(ctx, result.put());

    BOOST_REQUIRE_EQUAL(hr, S_OK);

    const FABRIC_NODE_QUERY_RESULT_LIST *nodes = result->get_NodeList();
    BOOST_CHECK_NE(nodes, nullptr);
  };

  belt::com::com_ptr<IFabricAsyncOperationCallback> callback =
      sf::AsioCallback::create_instance(lamda_callback,
                                        io_context.get_executor())
          .to_ptr();

  belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
  FABRIC_NODE_QUERY_DESCRIPTION node = {};
  hr = client->BeginGetNodeList(&node, 1000, callback.get(), ctx.put());
  BOOST_REQUIRE_EQUAL(hr, S_OK);

  io_context.run();
}

BOOST_AUTO_TEST_CASE(test_asio_waitable_callback) {

  belt::com::com_ptr<IFabricQueryClient> client;

  HRESULT hr =
      ::FabricCreateLocalClient(IID_IFabricQueryClient, (void **)client.put());

  BOOST_REQUIRE_EQUAL(hr, S_OK);

  auto f = [&]() -> net::awaitable<void> {
    auto executor = co_await net::this_coro::executor;
    HRESULT hr = S_OK;
    // this is a obj holder
    belt::com::com_ptr<sf::IAwaitableCallback> callback =
        sf::AsioAwaitableCallback::create_instance(executor).to_ptr();

    belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
    FABRIC_NODE_QUERY_DESCRIPTION node = {};
    hr = client->BeginGetNodeList(&node, 1000, callback.get(), ctx.put());

    BOOST_REQUIRE_EQUAL(hr, S_OK);
    co_await callback->await();

    belt::com::com_ptr<IFabricGetNodeListResult> result;
    hr = client->EndGetNodeList(ctx.get(), result.put());

    BOOST_REQUIRE_EQUAL(hr, S_OK);

    const FABRIC_NODE_QUERY_RESULT_LIST *nodes = result->get_NodeList();
    BOOST_CHECK_NE(nodes, nullptr);
  };
  net::io_context io_context;

  net::co_spawn(io_context, f, net::detached);
  net::co_spawn(io_context, f, net::detached);

  io_context.run();
}

BOOST_AUTO_TEST_CASE(test_asio_waitable_fabric_client) {
  belt::com::com_ptr<IFabricQueryClient> client;

  HRESULT hr =
      ::FabricCreateLocalClient(IID_IFabricQueryClient, (void **)client.put());

  BOOST_REQUIRE_EQUAL(hr, S_OK);

  // try use asio ptr
  boost::system::error_code ec;
  net::io_context io_context;

  sf::AwaitableFabricQueryClient fc(client);

  auto f = [&]() -> net::awaitable<void> {
    FABRIC_NODE_QUERY_DESCRIPTION node = {};
    belt::com::com_ptr<IFabricGetNodeListResult> result;
    HRESULT lhr = co_await fc.GetNodeListExample(&node, result.put());
    BOOST_REQUIRE_EQUAL(lhr, S_OK);
    BOOST_REQUIRE_NE(result->get_NodeList(), nullptr);
  };

  net::co_spawn(io_context, f, net::detached);

  auto f2 = [&]() -> net::awaitable<void> {
    HRESULT lhr = S_OK;
    FABRIC_APPLICATION_TYPE_QUERY_DESCRIPTION query = {};
    belt::com::com_ptr<IFabricGetApplicationTypeListResult> result;
    lhr = co_await fc.GetApplicationTypeList(&query, result.put());
    BOOST_REQUIRE_EQUAL(lhr, S_OK);
    BOOST_REQUIRE_NE(result->get_ApplicationTypeList(), nullptr);
  };

  net::co_spawn(io_context, f2, net::detached);

  belt::com::com_ptr<IFabricHealthClient> healthClient;

  hr = ::FabricCreateLocalClient(IID_IFabricHealthClient,
                                 (void **)healthClient.put());
  BOOST_REQUIRE_EQUAL(hr, S_OK);

  sf::AwaitableFabricHealthClient hc(healthClient);

  auto fhealth = [&]() -> net::awaitable<void> {
    HRESULT lhr = S_OK;
    FABRIC_CLUSTER_HEALTH_POLICY query = {};
    belt::com::com_ptr<IFabricClusterHealthResult> result;
    lhr = co_await hc.GetClusterHealth(&query, result.put());
    BOOST_REQUIRE_EQUAL(lhr, S_OK);
    BOOST_REQUIRE_NE(result->get_ClusterHealth(), nullptr);
  };
  net::co_spawn(io_context, fhealth, net::detached);

  auto fhealth2 = [&]() -> net::awaitable<void> {
    HRESULT lhr = S_OK;
    std::wstring nodeName = L"_Node_0"; // This is the name in default cluster
    FABRIC_CLUSTER_HEALTH_POLICY query = {};
    {
      belt::com::com_ptr<IFabricNodeHealthResult> result;
      lhr = co_await hc.GetNodeHealth(nodeName.c_str(), &query, result.put());
      BOOST_REQUIRE_EQUAL(lhr, S_OK);
      BOOST_REQUIRE_NE(result->get_NodeHealth(), nullptr);
    }
    // get a unknown node and check error
    {
      belt::com::com_ptr<IFabricNodeHealthResult> result;
      lhr = co_await hc.GetNodeHealth(L"BadNodeName", &query, result.put());
      BOOST_CHECK_MESSAGE(lhr == FABRIC_E_HEALTH_ENTITY_NOT_FOUND,
                          "not found: " + sf::get_fabric_error_str(lhr));
      BOOST_REQUIRE_EQUAL(lhr, FABRIC_E_HEALTH_ENTITY_NOT_FOUND);
      BOOST_REQUIRE_EQUAL(result.get(), nullptr);
    }
  };
  net::co_spawn(io_context, fhealth2, net::detached);

  io_context.run();
}

BOOST_AUTO_TEST_SUITE_END()

// Tests for reverse wrapper
// impl is coro but the interface is fabric async.
// This is an example of how to impl fabric async interface,
// but the internal is using asio coroutine.
// Similar technique can be used for winrt.

#include "FabricRuntime.h"

#include "servicefabric/string_result.hpp"
#include "servicefabric/waitable_callback.hpp"

#include <any>
#include <latch>

class myctx : public belt::com::object<myctx, IFabricAsyncOperationContext> {
public:
  myctx(){};
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
  belt::com::com_ptr<IFabricStringResult> addr;
};

class appInstanceImpl {
public:
  boost::asio::awaitable<HRESULT>
  Open(IFabricStatelessServicePartition *partition,
       IFabricStringResult **serviceAddress) {
    UNREFERENCED_PARAMETER(partition);
    *serviceAddress =
        sf::string_result::create_instance(L"myaddress").to_ptr().detach();
    co_return S_OK;
  }
  boost::asio::awaitable<HRESULT> Close() { co_return S_OK; }
};

class appInstance
    : public belt::com::object<appInstance, IFabricStatelessServiceInstance> {
public:
  appInstance(net::io_context &io_context) : io_context_(io_context) {}

  HRESULT STDMETHODCALLTYPE BeginOpen(
      /* [in] */ IFabricStatelessServicePartition *partition,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {

    belt::com::com_ptr<IFabricAsyncOperationContext> ctx =
        myctx::create_instance().to_ptr();
    belt::com::com_ptr<IFabricAsyncOperationCallback> cb(callback);

    auto f2 =
        [this](belt::com::com_ptr<IFabricStatelessServicePartition> fpartition,
               belt::com::com_ptr<IFabricAsyncOperationCallback> fcallback,
               belt::com::com_ptr<IFabricAsyncOperationContext> fctx)
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

    net::co_spawn(io_context_, std::bind(f2, partition, cb, ctx),
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

BOOST_AUTO_TEST_SUITE(test_fabric_asio2)

BOOST_AUTO_TEST_CASE(test_asio_fabric_reverse) {
  net::io_context ioc;

  belt::com::com_ptr<IFabricStatelessServiceInstance> svc =
      appInstance::create_instance(ioc).to_ptr();

  // ioc must run after the job is posted to it,
  // otherwise ioc will see no jobs and finish run immediately.
  std::latch lch(1);

  // use waitable ctx
  auto f = [&]() {
    belt::com::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        sf::FabricAsyncOperationWaitableCallback::create_instance().to_ptr();

    belt::com::com_ptr<IFabricAsyncOperationContext> ctx;

    HRESULT hr = svc->BeginOpen(nullptr, // partition
                                callback.get(), ctx.put());
    // job is pushed to ioc in svc
    lch.count_down();
    BOOST_REQUIRE_EQUAL(hr, S_OK);
    callback->Wait();
    belt::com::com_ptr<IFabricStringResult> addr;
    hr = svc->EndOpen(ctx.get(), addr.put());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
    BOOST_REQUIRE(std::wstring(addr->get_String()) ==
                  std::wstring(L"myaddress"));
  };

  // start the operation
  std::jthread th(f);

  std::jthread th_io([&]() {
    lch.wait();
    ioc.run();
  });

  th.join();
  th_io.join();
}

BOOST_AUTO_TEST_SUITE_END()