// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include <spdlog/spdlog.h>

#include <fabrictransport_.h>
#include <servicefabric/async_context.hpp>
#include <servicefabric/transport_message.hpp>
#include <winrt/base.h>

namespace sf = servicefabric;

// for server handling requests
class request_handler
    : public winrt::implements<request_handler,
                               IFabricTransportMessageHandler> {
public:
  HRESULT STDMETHODCALLTYPE BeginProcessRequest(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricTransportMessage *message,
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
    UNREFERENCED_PARAMETER(timeoutMilliseconds);
    spdlog::debug(L"request_handler::BeginProcessRequest id: {}", clientId);

    std::string body = sf::get_body(message);
    std::string headers = sf::get_header(message);
    spdlog::debug("request_handler::BeginProcessRequest header: {} body: {}",
                  headers, body);

    winrt::com_ptr<IFabricAsyncOperationContext> ctx =
        winrt::make<sf::async_context>(callback);
    *context = ctx.detach();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE EndProcessRequest(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricTransportMessage **reply) override {
    UNREFERENCED_PARAMETER(context);
    spdlog::debug("request_handler::EndProcessRequest");
    winrt::com_ptr<IFabricTransportMessage> msg1 =
        winrt::make<sf::transport_message>("mybodyreply", "myheaderreply");
    *reply = msg1.detach();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE HandleOneWay(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricTransportMessage *message) override {
    UNREFERENCED_PARAMETER(clientId);
    UNREFERENCED_PARAMETER(message);
    spdlog::debug("request_handler::HandleOneWay");
    return S_OK;
  }
};