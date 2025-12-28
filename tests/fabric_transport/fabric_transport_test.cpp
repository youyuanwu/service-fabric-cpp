// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include <boost/ut.hpp>

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

boost::ut::suite test_fabric_transport = [] {
  using namespace boost::ut;

  "test_1"_test = [] {
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

    expect(hr == S_OK >> fatal);
    winrt::com_ptr<IFabricStringResult> addr_str;
    {
      // open listener
      winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
          winrt::make<sf::FabricAsyncOperationWaitableCallback>();
      winrt::com_ptr<IFabricAsyncOperationContext> ctx;
      hr = listener->BeginOpen(callback.get(), ctx.put());
      expect(hr == S_OK >> fatal);
      callback->Wait();
      hr = listener->EndOpen(ctx.get(), addr_str.put());
      expect(hr == S_OK >> fatal);
    }
#ifdef SF_DEBUG
    std::wcout << "Server listening on : " << addr_str->get_String()
               << std::endl;
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
    expect(hr == S_OK);

    // open client
    {
      winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
          winrt::make<sf::FabricAsyncOperationWaitableCallback>();
      winrt::com_ptr<IFabricAsyncOperationContext> ctx;
      hr = client->BeginOpen(1000, callback.get(), ctx.put());
      expect(hr == S_OK >> fatal);
      callback->Wait();
      hr = client->EndOpen(ctx.get());
      expect(hr == S_OK >> fatal);
    }

    // sleep for some time
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // make request
    {
      winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
          winrt::make<sf::FabricAsyncOperationWaitableCallback>();
      winrt::com_ptr<IFabricAsyncOperationContext> ctx;
      winrt::com_ptr<IFabricTransportMessage> msg =
          winrt::make<sf::transport_message>("mybody", "myheader");
      hr = client->BeginRequest(msg.get(), 1000, callback.get(), ctx.put());
      expect(hr == S_OK >> fatal);
      callback->Wait();
      winrt::com_ptr<IFabricTransportMessage> reply;
      hr = client->EndRequest(ctx.get(), reply.put());
      expect(hr == S_OK >> fatal) << "Error: " << sf::get_fabric_error_str(hr);
      std::string body = sf::get_body(reply.get());
      std::string headers = sf::get_header(reply.get());
#ifdef SF_DEBUG
      std::cout << "reply header: " << headers << " body: " << body
                << std::endl;
#endif
    }

    // close listener
    {
      winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
          winrt::make<sf::FabricAsyncOperationWaitableCallback>();
      winrt::com_ptr<IFabricAsyncOperationContext> ctx;
      hr = listener->BeginClose(callback.get(), ctx.put());
      expect(hr == S_OK);
      callback->Wait();
      hr = listener->EndClose(ctx.get());
      expect(hr == S_OK);
    }
  };
};

int main() {}