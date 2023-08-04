// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "FabricClient.h"
#include "servicefabric/asio_callback.hpp"

namespace servicefabric {

namespace net = boost::asio;
namespace sf = servicefabric;

template <typename IClient, typename BeginFunc, typename EndFunc,
          typename RetType, typename... TParams>
boost::asio::awaitable<HRESULT> async_operation(IClient *client, BeginFunc bf,
                                                EndFunc ef, RetType **ret,
                                                TParams... params) {
  auto executor = co_await net::this_coro::executor;
  HRESULT hr = S_OK;
  // this is a obj holder
  winrt::com_ptr<sf::IAwaitableCallback> callback =
      winrt::make<sf::AsioAwaitableCallback>(executor);

  winrt::com_ptr<IFabricAsyncOperationContext> ctx;
  // FABRIC_NODE_QUERY_DESCRIPTION node = {};
  hr = (*client.*bf)(params..., 1000, callback.get(), ctx.put());
  if (hr != S_OK) {
    co_return hr;
  }

  co_await callback->await();

  hr = (*client.*ef)(ctx.get(), ret);
  co_return hr;
}

class AwaitableFabricQueryClient {
public:
  AwaitableFabricQueryClient(winrt::com_ptr<IFabricQueryClient> client)
      : client_(client) {}

  // async api
  boost::asio::awaitable<HRESULT>
  GetNodeList(const FABRIC_NODE_QUERY_DESCRIPTION *queryDescription,
              IFabricGetNodeListResult **ret);

  // example without template
  boost::asio::awaitable<HRESULT>
  GetNodeListExample(const FABRIC_NODE_QUERY_DESCRIPTION *queryDescription,
                     IFabricGetNodeListResult **ret);

  boost::asio::awaitable<HRESULT> GetApplicationTypeList(
      const FABRIC_APPLICATION_TYPE_QUERY_DESCRIPTION *queryDescription,
      IFabricGetApplicationTypeListResult **ret);

private:
  winrt::com_ptr<IFabricQueryClient> client_;
};

class AwaitableFabricHealthClient {
public:
  AwaitableFabricHealthClient(winrt::com_ptr<IFabricHealthClient> client)
      : client_(client) {}

  boost::asio::awaitable<HRESULT>
  GetClusterHealth(const FABRIC_CLUSTER_HEALTH_POLICY *queryDescription,
                   IFabricClusterHealthResult **ret);

  boost::asio::awaitable<HRESULT>
  GetNodeHealth(LPCWSTR nodeName,
                const FABRIC_CLUSTER_HEALTH_POLICY *queryDescription,
                IFabricNodeHealthResult **ret);

private:
  winrt::com_ptr<IFabricHealthClient> client_;
};

} // namespace servicefabric