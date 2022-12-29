#pragma once

#include "FabricClient.h"
#include "servicefabric/asio_callback.hpp"

namespace servicefabric {

namespace net = boost::asio;
namespace sf = servicefabric;

template <typename IClient, typename BeginFunc, typename EndFunc,
          typename RetType, typename... TParams>
boost::asio::awaitable<void> async_operation(IClient *client, BeginFunc bf,
                                             EndFunc ef, HRESULT *hr,
                                             RetType **ret, TParams... params) {
  auto executor = co_await net::this_coro::executor;
  HRESULT lhr = S_OK;
  // this is a obj holder
  belt::com::com_ptr<sf::IAwaitableCallback> callback =
      sf::AsioAwaitableCallback::create_instance(executor).to_ptr();

  belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
  // FABRIC_NODE_QUERY_DESCRIPTION node = {};
  lhr = (*client.*bf)(params..., 1000, callback.get(), ctx.put());
  if (lhr != S_OK) {
    *hr = lhr;
    co_return;
  }

  co_await callback->await();

  lhr = (*client.*ef)(ctx.get(), ret);
  if (lhr != S_OK) {
    *hr = lhr;
    co_return;
  }
}

class AwaitableFabricQueryClient {
public:
  AwaitableFabricQueryClient(belt::com::com_ptr<IFabricQueryClient> client)
      : client_(client) {}

  // async api
  boost::asio::awaitable<void>
  GetNodeList(HRESULT *hr,
              const FABRIC_NODE_QUERY_DESCRIPTION *queryDescription,
              IFabricGetNodeListResult **ret);

  // example without template
  boost::asio::awaitable<void>
  GetNodeListExample(HRESULT *hr,
                     const FABRIC_NODE_QUERY_DESCRIPTION *queryDescription,
                     IFabricGetNodeListResult **ret);

  boost::asio::awaitable<void> GetApplicationTypeList(
      HRESULT *hr,
      const FABRIC_APPLICATION_TYPE_QUERY_DESCRIPTION *queryDescription,
      IFabricGetApplicationTypeListResult **ret);

private:
  belt::com::com_ptr<IFabricQueryClient> client_;
};

class AwaitableFabricHealthClient {
public:
  AwaitableFabricHealthClient(belt::com::com_ptr<IFabricHealthClient> client)
      : client_(client) {}

  boost::asio::awaitable<void>
  GetClusterHealth(HRESULT *hr,
                   const FABRIC_CLUSTER_HEALTH_POLICY *queryDescription,
                   IFabricClusterHealthResult **ret);

  boost::asio::awaitable<void>
  GetNodeHealth(HRESULT *hr, LPCWSTR nodeName,
                const FABRIC_CLUSTER_HEALTH_POLICY *queryDescription,
                IFabricNodeHealthResult **ret);

private:
  belt::com::com_ptr<IFabricHealthClient> client_;
};

} // namespace servicefabric