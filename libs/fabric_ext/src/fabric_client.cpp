#include "servicefabric/fabric_client.hpp"

namespace servicefabric {

namespace net = boost::asio;
namespace sf = servicefabric;

boost::asio::awaitable<void> AwaitableFabricQueryClient::GetNodeListExample(
    HRESULT *hr, const FABRIC_NODE_QUERY_DESCRIPTION *queryDescription,
    IFabricGetNodeListResult **result) {
  assert(hr != nullptr);
  assert(queryDescription != nullptr);
  assert(result != nullptr);

  auto executor = co_await net::this_coro::executor;
  HRESULT lhr = S_OK;
  // this is a obj holder
  belt::com::com_ptr<sf::IAwaitableCallback> callback =
      sf::AsioAwaitableCallback::create_instance(executor).to_ptr();

  belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
  FABRIC_NODE_QUERY_DESCRIPTION node = {};
  lhr = client_->BeginGetNodeList(&node, 1000, callback.get(), ctx.put());
  if (lhr != S_OK) {
    *hr = lhr;
    co_return;
  }

  // BOOST_REQUIRE_EQUAL(hr, S_OK);
  co_await callback->await();

  lhr = client_->EndGetNodeList(ctx.get(), result);
  if (lhr != S_OK) {
    *hr = lhr;
    co_return;
  }
}

boost::asio::awaitable<void> AwaitableFabricQueryClient::GetNodeList(
    HRESULT *hr, const FABRIC_NODE_QUERY_DESCRIPTION *queryDescription,
    IFabricGetNodeListResult **ret) {
  co_await sf::async_operation(
      client_.get(), &IFabricQueryClient::BeginGetNodeList,
      &IFabricQueryClient::EndGetNodeList, hr, ret, queryDescription);
}

boost::asio::awaitable<void> AwaitableFabricQueryClient::GetApplicationTypeList(
    HRESULT *hr,
    const FABRIC_APPLICATION_TYPE_QUERY_DESCRIPTION *queryDescription,
    IFabricGetApplicationTypeListResult **ret) {
  co_await sf::async_operation(client_.get(),
                               &IFabricQueryClient::BeginGetApplicationTypeList,
                               &IFabricQueryClient::EndGetApplicationTypeList,
                               hr, ret, queryDescription);
}

boost::asio::awaitable<void> AwaitableFabricHealthClient::GetClusterHealth(
    HRESULT *hr, const FABRIC_CLUSTER_HEALTH_POLICY *queryDescription,
    IFabricClusterHealthResult **ret) {
  co_await sf::async_operation(
      client_.get(), &IFabricHealthClient::BeginGetClusterHealth,
      &IFabricHealthClient::EndGetClusterHealth, hr, ret, queryDescription);
}

boost::asio::awaitable<void> AwaitableFabricHealthClient::GetNodeHealth(
    HRESULT *hr, LPCWSTR nodeName,
    const FABRIC_CLUSTER_HEALTH_POLICY *queryDescription,
    IFabricNodeHealthResult **ret) {
  co_await sf::async_operation(client_.get(),
                               &IFabricHealthClient::BeginGetNodeHealth,
                               &IFabricHealthClient::EndGetNodeHealth, hr, ret,
                               nodeName, queryDescription);
}

} // namespace servicefabric