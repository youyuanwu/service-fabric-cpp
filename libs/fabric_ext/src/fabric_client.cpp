#include "servicefabric/fabric_client.hpp"

namespace servicefabric {

namespace net = boost::asio;
namespace sf = servicefabric;

boost::asio::awaitable<HRESULT> AwaitableFabricQueryClient::GetNodeListExample(
    const FABRIC_NODE_QUERY_DESCRIPTION *queryDescription,
    IFabricGetNodeListResult **result) {
  assert(queryDescription != nullptr);
  assert(result != nullptr);

  auto executor = co_await net::this_coro::executor;
  HRESULT hr = S_OK;

  belt::com::com_ptr<sf::IAwaitableCallback> callback =
      sf::AsioAwaitableCallback::create_instance(executor).to_ptr();

  belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
  FABRIC_NODE_QUERY_DESCRIPTION node = {};
  hr = client_->BeginGetNodeList(&node, 1000, callback.get(), ctx.put());
  if (hr != S_OK) {
    co_return hr;
  }

  co_await callback->await();

  hr = client_->EndGetNodeList(ctx.get(), result);
  if (hr != S_OK) {
    co_return hr;
  }
  co_return S_OK;
}

boost::asio::awaitable<HRESULT> AwaitableFabricQueryClient::GetNodeList(
    const FABRIC_NODE_QUERY_DESCRIPTION *queryDescription,
    IFabricGetNodeListResult **ret) {
  return sf::async_operation(
      client_.get(), &IFabricQueryClient::BeginGetNodeList,
      &IFabricQueryClient::EndGetNodeList, ret, queryDescription);
}

boost::asio::awaitable<HRESULT>
AwaitableFabricQueryClient::GetApplicationTypeList(
    const FABRIC_APPLICATION_TYPE_QUERY_DESCRIPTION *queryDescription,
    IFabricGetApplicationTypeListResult **ret) {
  return sf::async_operation(
      client_.get(), &IFabricQueryClient::BeginGetApplicationTypeList,
      &IFabricQueryClient::EndGetApplicationTypeList, ret, queryDescription);
}

boost::asio::awaitable<HRESULT> AwaitableFabricHealthClient::GetClusterHealth(
    const FABRIC_CLUSTER_HEALTH_POLICY *queryDescription,
    IFabricClusterHealthResult **ret) {
  return sf::async_operation(
      client_.get(), &IFabricHealthClient::BeginGetClusterHealth,
      &IFabricHealthClient::EndGetClusterHealth, ret, queryDescription);
}

boost::asio::awaitable<HRESULT> AwaitableFabricHealthClient::GetNodeHealth(
    LPCWSTR nodeName, const FABRIC_CLUSTER_HEALTH_POLICY *queryDescription,
    IFabricNodeHealthResult **ret) {
  return sf::async_operation(
      client_.get(), &IFabricHealthClient::BeginGetNodeHealth,
      &IFabricHealthClient::EndGetNodeHealth, ret, nodeName, queryDescription);
}

} // namespace servicefabric