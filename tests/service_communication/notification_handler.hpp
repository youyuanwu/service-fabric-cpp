// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "fabricservicecommunication_.h"
#include "servicefabric/async_context.hpp"
#include <winrt/base.h>

#include "message.hpp"

namespace sf = servicefabric;

// server handle request
class notification_handler
    : public winrt::implements<notification_handler,
                               IFabricCommunicationMessageHandler> {
public:
  HRESULT STDMETHODCALLTYPE BeginProcessRequest(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricServiceCommunicationMessage *message,
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
    UNREFERENCED_PARAMETER(clientId);
    UNREFERENCED_PARAMETER(timeoutMilliseconds);
    FABRIC_MESSAGE_BUFFER *body = message->Get_Body();
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "notification_handler::BeginProcessRequest: body "
        << std::string(body->Buffer, body->Buffer + body->BufferSize);
#endif
    winrt::com_ptr<IFabricAsyncOperationContext> ctx =
        winrt::make<sf::async_context>(callback);
    *context = ctx.detach();
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE EndProcessRequest(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricServiceCommunicationMessage **reply) override {
    UNREFERENCED_PARAMETER(context);
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "notification_handler::EndProcessRequest";
#endif
    winrt::com_ptr<IFabricServiceCommunicationMessage> msg1 =
        winrt::make<message>("mybodyreply", "myheaderreply");
    *reply = msg1.detach();
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE HandleOneWay(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricServiceCommunicationMessage *message) override {
    UNREFERENCED_PARAMETER(clientId);
    UNREFERENCED_PARAMETER(message);
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "notification_handler::HandleOneWay";
#endif
    return S_OK;
  }
};