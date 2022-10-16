#pragma once

#include <boost/log/trivial.hpp>

#include <fabrictransport_.h>
#include <moderncom/interfaces.h>
#include <servicefabric/async_context.hpp>

#include "message.hpp"

namespace sf = servicefabric;

// for server handling requests
class request_handler
    : public belt::com::object<request_handler,
                               IFabricTransportMessageHandler> {
public:
  HRESULT STDMETHODCALLTYPE BeginProcessRequest(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricTransportMessage *message,
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "request_handler::BeginProcessRequest id: " << clientId;
#endif
    std::string body;
    std::string headers;
    if (message != nullptr) {
      const FABRIC_TRANSPORT_MESSAGE_BUFFER *headerbuf = {};
      const FABRIC_TRANSPORT_MESSAGE_BUFFER *msgbuf = {};
      ULONG msgcount = 0;
      message->GetHeaderAndBodyBuffer(&headerbuf, &msgcount, &msgbuf);
      headers = std::string(headerbuf->Buffer,
                            headerbuf->Buffer + headerbuf->BufferSize);
      for (std::size_t i = 0; i < msgcount; i++) {
        const FABRIC_TRANSPORT_MESSAGE_BUFFER *msg_i = msgbuf + i;
        std::string msg_str(msg_i->Buffer, msg_i->Buffer + msg_i->BufferSize);
        body += msg_str;
      }
    }
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "request_handler::BeginProcessRequest header: " << headers
        << " body: " << body;
#endif

    belt::com::com_ptr<IFabricAsyncOperationContext> ctx =
        sf::async_context::create_instance(callback).to_ptr();
    *context = ctx.detach();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE EndProcessRequest(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricTransportMessage **reply) override {

#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "request_handler::EndProcessRequest";
#endif
    belt::com::com_ptr<IFabricTransportMessage> msg1 =
        message::create_instance("mybodyreply", "myheaderreply").to_ptr();
    *reply = msg1.detach();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE HandleOneWay(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricTransportMessage *message) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "request_handler::HandleOneWay";
#endif
    return S_OK;
  }
};