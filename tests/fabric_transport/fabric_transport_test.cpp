#define BOOST_TEST_MODULE service_communication

#include <boost/test/unit_test.hpp>

#include "fabrictransport_.h"

#include <moderncom/interfaces.h>

#include "conn_handler.hpp"
#include "msg_disposer.hpp"
#include "request_handler.hpp"

#include "client_event_handler.hpp"
#include "client_notification_handler.hpp"

#include "servicefabric/async_context.hpp"
#include "servicefabric/fabric_error.hpp"
#include "servicefabric/waitable_callback.hpp"
namespace sf = servicefabric;

BOOST_AUTO_TEST_SUITE(test_fabric_transport)

BOOST_AUTO_TEST_CASE(test_1) {

  FABRIC_SECURITY_CREDENTIALS cred = {};
  cred.Kind = FABRIC_SECURITY_CREDENTIAL_KIND_NONE;

  FABRIC_TRANSPORT_SETTINGS settings = {};
  settings.KeepAliveTimeoutInSeconds = 10;
  settings.MaxConcurrentCalls = 10;
  settings.MaxMessageSize = 100;
  settings.MaxQueueSize = 100;
  settings.OperationTimeoutInSeconds = 30;
  settings.SecurityCredentials = &cred;

  FABRIC_TRANSPORT_LISTEN_ADDRESS addr = {};
  addr.IPAddressOrFQDN = L"localhost";
  addr.Path = L"/";
  addr.Port = 12345;

  belt::com::com_ptr<IFabricTransportMessageHandler> req_handler =
      request_handler::create_instance().to_ptr();
  belt::com::com_ptr<IFabricTransportConnectionHandler> conn_handler =
      conn_handler::create_instance().to_ptr();
  belt::com::com_ptr<IFabricTransportMessageDisposer> msg_disposer =
      msg_disposer::create_instance().to_ptr();
  belt::com::com_ptr<IFabricTransportListener> listener;

  // create listener
  HRESULT hr = CreateFabricTransportListener(
      IID_IFabricTransportListener, &settings, &addr, req_handler.get(),
      conn_handler.get(), msg_disposer.get(), listener.put());

  BOOST_REQUIRE_EQUAL(S_OK, hr);
  // open listener
  belt::com::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
      sf::FabricAsyncOperationWaitableCallback::create_instance().to_ptr();
  belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
  hr = listener->BeginOpen(callback.get(), ctx.put());
  BOOST_REQUIRE_EQUAL(hr, S_OK);
  callback->Wait();
  belt::com::com_ptr<IFabricStringResult> addr_str;
  hr = listener->EndOpen(ctx.get(), addr_str.put());
  BOOST_REQUIRE_EQUAL(hr, S_OK);

#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "Server listening on : "
                           << addr_str->get_String();
#endif

  belt::com::com_ptr<IFabricTransportCallbackMessageHandler> client_notify_h =
      client_notification_handler::create_instance().to_ptr();

  belt::com::com_ptr<IFabricTransportClientEventHandler> client_event_h =
      client_event_handler::create_instance().to_ptr();
  belt::com::com_ptr<IFabricTransportMessageDisposer> client_msg_disposer =
      msg_disposer::create_instance().to_ptr();
  ;
  belt::com::com_ptr<IFabricTransportClient> client;

  // open client
  hr = CreateFabricTransportClient(
      /* [in] */ IID_IFabricTransportClient,
      /* [in] */ &settings,
      /* [in] */ addr_str->get_String(),
      /* [in] */ client_notify_h.get(),
      /* [in] */ client_event_h.get(),
      /* [in] */ client_msg_disposer.get(),
      /* [retval][out] */ client.put());
  BOOST_REQUIRE_EQUAL(hr, S_OK);

  // open client
  {
    belt::com::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        sf::FabricAsyncOperationWaitableCallback::create_instance().to_ptr();
    belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
    hr = client->BeginOpen(1000, callback.get(), ctx.put());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
    callback->Wait();
    hr = client->EndOpen(ctx.get());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
  }

  // make request
  {
    belt::com::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        sf::FabricAsyncOperationWaitableCallback::create_instance().to_ptr();
    belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
    belt::com::com_ptr<IFabricTransportMessage> msg =
        message::create_instance("mybody", "myheader").to_ptr();
    hr = client->BeginRequest(msg.get(), 1000, callback.get(), ctx.put());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
    callback->Wait();
    belt::com::com_ptr<IFabricTransportMessage> reply;
    hr = client->EndRequest(ctx.get(), reply.put());
    BOOST_REQUIRE_EQUAL(hr, S_OK);

    std::string body;
    std::string headers;
    const FABRIC_TRANSPORT_MESSAGE_BUFFER *headerbuf = {};
    const FABRIC_TRANSPORT_MESSAGE_BUFFER *msgbuf = {};
    ULONG msgcount = 0;
    reply->GetHeaderAndBodyBuffer(&headerbuf, &msgcount, &msgbuf);
    headers = std::string(headerbuf->Buffer,
                          headerbuf->Buffer + headerbuf->BufferSize);
    for (std::size_t i = 0; i < msgcount; i++) {
      const FABRIC_TRANSPORT_MESSAGE_BUFFER *msg_i = msgbuf + i;
      std::string msg_str(msg_i->Buffer, msg_i->Buffer + msg_i->BufferSize);
      body += msg_str;
    }
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "reply header: " << headers << " body: " << body;
#endif
  }

  // close listener
  {
    belt::com::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        sf::FabricAsyncOperationWaitableCallback::create_instance().to_ptr();
    belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
    hr = listener->BeginClose(callback.get(), ctx.put());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
    hr = listener->EndClose(ctx.get());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
  }
}

BOOST_AUTO_TEST_SUITE_END()