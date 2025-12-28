// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include <spdlog/spdlog.h>

#include <FabricClient.h>

#include <servicefabric/asio_callback.hpp>
#include <servicefabric/fabric_error.hpp>

namespace sf = servicefabric;
namespace net = asio;

int main() {

  winrt::com_ptr<IFabricQueryClient> client;

  HRESULT hr =
      ::FabricCreateLocalClient(IID_IFabricQueryClient, (void **)client.put());

  if (hr != NO_ERROR) {
    spdlog::debug("client creation failed. {}", hr);
    return EXIT_FAILURE;
  }

  spdlog::debug("FabricCreateLocalClient success");

  // try use asio ptr
  asio::error_code ec;
  net::io_context io_context;

  auto lamda_callback = [client](IFabricAsyncOperationContext *ctx) {
    winrt::com_ptr<IFabricGetNodeListResult> result;
    HRESULT hr = client->EndGetNodeList(ctx, result.put());
    if (hr != NO_ERROR) {
      spdlog::debug("EndGetNodeList failed: {} {}", hr,
                    sf::get_fabric_error_str(hr));
      return;
    }
    const FABRIC_NODE_QUERY_RESULT_LIST *nodes = result->get_NodeList();
    if (nodes != nullptr) {
      spdlog::debug("node count: {}", nodes->Count);
    }
  };

  winrt::com_ptr<IFabricAsyncOperationCallback> callback =
      winrt::make<sf::AsioCallback>(lamda_callback, io_context.get_executor());

  winrt::com_ptr<IFabricAsyncOperationContext> ctx;
  FABRIC_NODE_QUERY_DESCRIPTION node = {};
  hr = client->BeginGetNodeList(&node, 1000, callback.get(), ctx.put());
  if (hr != NO_ERROR) {
    spdlog::debug("BeginGetNodeList failed: {} {}", hr,
                  sf::get_fabric_error_str(hr));
    return EXIT_FAILURE;
  }

  io_context.run();
}