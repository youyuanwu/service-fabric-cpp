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