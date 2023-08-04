// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#define BOOST_TEST_MODULE service_communication

#include <boost/test/unit_test.hpp>

#include "fabrictransport_.h"

#include "request_handler.hpp"

#include "servicefabric/async_context.hpp"
#include "servicefabric/fabric_error.hpp"
#include "servicefabric/transport_dummy_client_conn_handler.hpp"
#include "servicefabric/transport_dummy_client_notification_handler.hpp"
#include "servicefabric/transport_dummy_msg_disposer.hpp"
#include "servicefabric/transport_dummy_server_conn_handler.hpp"
#include "servicefabric/transport_message.hpp"
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

  winrt::com_ptr<IFabricTransportMessageHandler> req_handler =
      winrt::make<request_handler>();
  winrt::com_ptr<IFabricTransportConnectionHandler> conn_handler =
      winrt::make<sf::transport_dummy_server_conn_handler>();
  winrt::com_ptr<IFabricTransportMessageDisposer> msg_disposer =
      winrt::make<sf::transport_dummy_msg_disposer>();
  winrt::com_ptr<IFabricTransportListener> listener;

  // create listener
  HRESULT hr = CreateFabricTransportListener(
      IID_IFabricTransportListener, &settings, &addr, req_handler.get(),
      conn_handler.get(), msg_disposer.get(), listener.put());

  BOOST_REQUIRE_EQUAL(S_OK, hr);
  winrt::com_ptr<IFabricStringResult> addr_str;
  {
    // open listener
    winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        winrt::make<sf::FabricAsyncOperationWaitableCallback>();
    winrt::com_ptr<IFabricAsyncOperationContext> ctx;
    hr = listener->BeginOpen(callback.get(), ctx.put());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
    callback->Wait();
    hr = listener->EndOpen(ctx.get(), addr_str.put());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
  }
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "Server listening on : "
                           << addr_str->get_String();
#endif

  winrt::com_ptr<IFabricTransportCallbackMessageHandler> client_notify_h =
      winrt::make<sf::transport_dummy_client_notification_handler>();

  winrt::com_ptr<IFabricTransportClientEventHandler> client_event_h =
      winrt::make<sf::transport_dummy_client_conn_handler>();
  winrt::com_ptr<IFabricTransportMessageDisposer> client_msg_disposer =
      winrt::make<sf::transport_dummy_msg_disposer>();
  winrt::com_ptr<IFabricTransportClient> client;

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
    winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        winrt::make<sf::FabricAsyncOperationWaitableCallback>();
    winrt::com_ptr<IFabricAsyncOperationContext> ctx;
    hr = client->BeginOpen(1000, callback.get(), ctx.put());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
    callback->Wait();
    hr = client->EndOpen(ctx.get());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
  }

  // make request
  {
    winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        winrt::make<sf::FabricAsyncOperationWaitableCallback>();
    winrt::com_ptr<IFabricAsyncOperationContext> ctx;
    winrt::com_ptr<IFabricTransportMessage> msg =
        winrt::make<sf::transport_message>("mybody", "myheader");
    hr = client->BeginRequest(msg.get(), 1000, callback.get(), ctx.put());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
    callback->Wait();
    winrt::com_ptr<IFabricTransportMessage> reply;
    hr = client->EndRequest(ctx.get(), reply.put());
    BOOST_REQUIRE_EQUAL(hr, S_OK);

    std::string body = sf::get_body(reply.get());
    std::string headers = sf::get_header(reply.get());
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "reply header: " << headers << " body: " << body;
#endif
  }

  // close listener
  {
    winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        winrt::make<sf::FabricAsyncOperationWaitableCallback>();
    winrt::com_ptr<IFabricAsyncOperationContext> ctx;
    hr = listener->BeginClose(callback.get(), ctx.put());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
    callback->Wait();
    hr = listener->EndClose(ctx.get());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
  }
}

BOOST_AUTO_TEST_SUITE_END()