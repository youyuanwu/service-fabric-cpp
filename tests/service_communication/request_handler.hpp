#pragma once

#include "fabricservicecommunication_.h"
#include <moderncom/interfaces.h>

#include "message.hpp"

// for server handling requests
class request_handler
    : public belt::com::object<request_handler,
                               IFabricCommunicationMessageHandler> {
public:
  HRESULT STDMETHODCALLTYPE BeginProcessRequest(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricServiceCommunicationMessage *message,
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
    FABRIC_MESSAGE_BUFFER *body = message->Get_Body();
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "request_handler::BeginProcessRequest: body "
        << std::string(body->Buffer, body->Buffer + body->BufferSize);
#endif
    belt::com::com_ptr<IFabricAsyncOperationContext> ctx =
        sf::async_context::create_instance(callback).to_ptr();
    *context = ctx.detach();
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE EndProcessRequest(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricServiceCommunicationMessage **reply) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "request_handler::EndProcessRequest";
#endif
    belt::com::com_ptr<IFabricServiceCommunicationMessage> msg1 =
        message::create_instance("mybodyreply", "myheaderreply").to_ptr();
    *reply = msg1.detach();
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE HandleOneWay(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricServiceCommunicationMessage *message) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "request_handler::HandleOneWay";
#endif
    return S_OK;
  }
};