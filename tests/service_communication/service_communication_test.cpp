// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include <boost/ut.hpp>
#include <iostream>

#include "fabricservicecommunication_.h"

#include "servicefabric/async_context.hpp"
#include "servicefabric/fabric_error.hpp"
#include "servicefabric/waitable_callback.hpp"

#include "conn_handler.hpp"
#include "request_handler.hpp"

#include "conn_event_handler.hpp"
#include "notification_handler.hpp"

namespace sf = servicefabric;

boost::ut::suite test_fabric_communication = [] {
  using namespace boost::ut;

  "test_construct"_test = [] {
    // create server
    FABRIC_SECURITY_CREDENTIALS cred = {};
    cred.Kind = FABRIC_SECURITY_CREDENTIAL_KIND_NONE;

    //   std::wstring const remoteName = L"localhost";
    //   LPCWSTR * remoteNames = new LPCWSTR[1];
    //   remoteNames[0] = remoteName.c_str();
    //   cred.Kind = FABRIC_SECURITY_CREDENTIAL_KIND_X509;
    //   FABRIC_X509_CREDENTIALS x509Credentials = {};
    //   x509Credentials.FindType = FABRIC_X509_FIND_TYPE_FINDBYSUBJECTNAME;
    //   x509Credentials.FindValue = (void*)L"CN=localhost";
    //   x509Credentials.StoreName = L"MY";
    //   x509Credentials.ProtectionLevel =
    //   FABRIC_PROTECTION_LEVEL_ENCRYPTANDSIGN; x509Credentials.StoreLocation =
    //   FABRIC_X509_STORE_LOCATION_LOCALMACHINE;
    //   x509Credentials.AllowedCommonNameCount = 1;
    //   x509Credentials.AllowedCommonNames = remoteNames;
    //   cred.Value = &x509Credentials;

    FABRIC_SERVICE_TRANSPORT_SETTINGS settings = {};
    settings.KeepAliveTimeoutInSeconds = 30;
    settings.MaxConcurrentCalls = 10;
    settings.MaxMessageSize = 10;
    settings.MaxQueueSize = 10;
    settings.OperationTimeoutInSeconds = 30;
    settings.SecurityCredentials = &cred;
    FABRIC_SERVICE_LISTENER_ADDRESS addr = {};
    addr.IPAddressOrFQDN = L"localhost";
    addr.Path = L"/";
    addr.Port = 12345;

    winrt::com_ptr<IFabricCommunicationMessageHandler> req_handler =
        winrt::make<request_handler>();
    winrt::com_ptr<IFabricServiceConnectionHandler> conn_handler_ptr =
        winrt::make<conn_handler>();

    winrt::com_ptr<IFabricServiceCommunicationListener> listener;

    HRESULT hr = CreateServiceCommunicationListener(
        IID_IFabricServiceCommunicationListener, &settings, &addr,
        req_handler.get(), conn_handler_ptr.get(), listener.put());

    expect(hr == S_OK) << sf::get_fabric_error_str(hr);
    winrt::com_ptr<IFabricStringResult> addr_str;
    {
      winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
          winrt::make<sf::FabricAsyncOperationWaitableCallback>();
      winrt::com_ptr<IFabricAsyncOperationContext> ctx;
      hr = listener->BeginOpen(callback.get(), ctx.put());
      expect(hr == S_OK);
      callback->Wait();
      hr = listener->EndOpen(ctx.get(), addr_str.put());
      expect(hr == S_OK);
    }
    spdlog::debug(L"Server listening on : {}", addr_str->get_String());
    // create client

    // This is server to client notification?
    // not seeing this being invoked. Some internal impl pass nullptr?
    winrt::com_ptr<IFabricCommunicationMessageHandler>
        notification_handler_ptr = winrt::make<notification_handler>();
    winrt::com_ptr<IFabricServiceConnectionEventHandler>
        conn_event_handler_ptr = winrt::make<conn_event_handler>();

    winrt::com_ptr<IFabricServiceCommunicationClient> client;
    hr = CreateServiceCommunicationClient(
        /* [in] */ IID_IFabricServiceCommunicationClient,
        /* [in] */ &settings,
        /* [in] */ L"localhost:12345+/",
        /* [in] */ notification_handler_ptr.get(),
        /* [in] */ conn_event_handler_ptr.get(), client.put());
    expect(hr == S_OK);

    // let server be up
    Sleep(3000);

    { // make request
      winrt::com_ptr<IFabricServiceCommunicationMessage> msg1 =
          winrt::make<message>("mybody", "myheader");
      winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
          winrt::make<sf::FabricAsyncOperationWaitableCallback>();
      winrt::com_ptr<IFabricAsyncOperationContext> ctx;
      hr = client->BeginRequest(msg1.get(), 1000, callback.get(), ctx.put());
      expect(hr == S_OK);
      callback->Wait();
      winrt::com_ptr<IFabricServiceCommunicationMessage> reply1;
      hr = client->EndRequest(ctx.get(), reply1.put());
      expect(hr == S_OK);

      FABRIC_MESSAGE_BUFFER *body = reply1->Get_Body();
      spdlog::debug("got reply: {}",
                    std::string(body->Buffer, body->Buffer + body->BufferSize));
    }
    // client->SendMessageA()

    // cleanup server
    {
      winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
          winrt::make<sf::FabricAsyncOperationWaitableCallback>();
      winrt::com_ptr<IFabricAsyncOperationContext> ctx;
      hr = listener->BeginClose(callback.get(), ctx.put());
      expect(hr == S_OK);
      hr = listener->EndClose(ctx.get());
      expect(hr == S_OK);
    }
  };
};

int main() {}