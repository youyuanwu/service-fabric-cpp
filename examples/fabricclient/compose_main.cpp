// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include <spdlog/spdlog.h>

#include <FabricClient.h>

#include <iostream>
#include <string>

#include <servicefabric/asio_callback.hpp>
#include <servicefabric/compose.hpp>

#include <thread>

namespace sf = servicefabric;
namespace net = boost::asio;

int main() {

  winrt::com_ptr<IFabricQueryClient> client;

  std::wstring conn = L"hello";
  HRESULT hr =
      ::FabricCreateLocalClient(IID_IFabricQueryClient, (void **)client.put());

  if (hr != NO_ERROR) {
    spdlog::error("FabricCreateLocalClient failed: {}", hr);
    return EXIT_FAILURE;
  }

  spdlog::info("FabricCreateLocalClient success");

  // try use asio ptr
  boost::system::error_code ec;
  net::io_context io_context;

  // node request
  sf::compose_op op(io_context.get_executor(), client,
                    &IFabricQueryClient::BeginGetNodeList,
                    &IFabricQueryClient::EndGetNodeList);

  FABRIC_NODE_QUERY_DESCRIPTION node_query = {};
  std::function<void(boost::system::error_code, IFabricGetNodeListResult *)>
      node_callback = [](boost::system::error_code ec,
                         IFabricGetNodeListResult *result) {
        spdlog::info("node_callback {}", ec.message());
        if (ec) {
          return;
        }
        const FABRIC_NODE_QUERY_RESULT_LIST *nodeList = result->get_NodeList();
        spdlog::info("node count {}", nodeList->Count);
        auto nodeItems = nodeList->Items;
        for (std::size_t i = 0; i < nodeList->Count; i++) {
          auto nodeItem = nodeItems + i;
          spdlog::info(L"node name: {} status: {} version: {}",
                       nodeItem->NodeName,
                       static_cast<int>(nodeItem->NodeStatus),
                       std::wstring(nodeItem->CodeVersion));
        }
      };
  op.async_exec(&node_query, node_callback);

  // run request one by one
  io_context.run();
  io_context.restart();

  // app request
  sf::compose_op op2(io_context.get_executor(), client,
                     &IFabricQueryClient::BeginGetApplicationTypeList,
                     &IFabricQueryClient::EndGetApplicationTypeList);

  FABRIC_APPLICATION_TYPE_QUERY_DESCRIPTION app_query = {};
  std::function<void(boost::system::error_code ec,
                     IFabricGetApplicationTypeListResult * result)>
      app_callback = [](boost::system::error_code ec,
                        IFabricGetApplicationTypeListResult *result) {
        spdlog::info("app_callback {}", ec.message());
        if (ec) {
          return;
        }
        auto list = result->get_ApplicationTypeList();
        auto list_count = list->Count;
        auto list_items = list->Items;
        for (std::size_t i = 0; i < list_count; i++) {
          auto item = list_items + i;
          spdlog::info(L"apptype name: {}", item->ApplicationTypeName);
        }
      };
  op2.async_exec(&app_query, app_callback);

  io_context.run();
}