#pragma once

#include <boost/log/trivial.hpp>

#include <fabrictransport_.h>
#include <moderncom/interfaces.h>
#include <servicefabric/async_context.hpp>
#include <servicefabric/transport_message.hpp>

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
    UNREFERENCED_PARAMETER(timeoutMilliseconds);
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "request_handler::BeginProcessRequest id: " << clientId;
#endif
    std::string body = sf::get_body(message);
    std::string headers = sf::get_header(message);
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
    UNREFERENCED_PARAMETER(context);
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "request_handler::EndProcessRequest";
#endif
    belt::com::com_ptr<IFabricTransportMessage> msg1 =
        sf::transport_message::create_instance("mybodyreply", "myheaderreply")
            .to_ptr();
    *reply = msg1.detach();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE HandleOneWay(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricTransportMessage *message) override {
    UNREFERENCED_PARAMETER(clientId);
    UNREFERENCED_PARAMETER(message);
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "request_handler::HandleOneWay";
#endif
    return S_OK;
  }
};