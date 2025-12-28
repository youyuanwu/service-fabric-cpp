// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "boost/log/trivial.hpp"
#include "fabricservicecommunication_.h"
#include <winrt/base.h>

#include "message.hpp"

// for server handling requests
class request_handler
    : public winrt::implements<request_handler,
                               IFabricCommunicationMessageHandler> {
public:
  HRESULT STDMETHODCALLTYPE BeginProcessRequest(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricServiceCommunicationMessage *message,
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
    UNREFERENCED_PARAMETER(timeoutMilliseconds);
    UNREFERENCED_PARAMETER(clientId);
#ifdef SF_DEBUG
    FABRIC_MESSAGE_BUFFER *body = message->Get_Body();
    BOOST_LOG_TRIVIAL(debug)
        << "request_handler::BeginProcessRequest: body "
        << std::string(body->Buffer, body->Buffer + body->BufferSize);
#else
    UNREFERENCED_PARAMETER(message);
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
    BOOST_LOG_TRIVIAL(debug) << "request_handler::EndProcessRequest";
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
    BOOST_LOG_TRIVIAL(debug) << "request_handler::HandleOneWay";
#endif
    return S_OK;
  }
};