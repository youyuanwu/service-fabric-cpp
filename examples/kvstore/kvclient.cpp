// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "fabrictransport_.h"

#include <moderncom/interfaces.h>

#include "servicefabric/async_context.hpp"
#include "servicefabric/fabric_error.hpp"
#include "servicefabric/transport_dummy_client_conn_handler.hpp"
#include "servicefabric/transport_dummy_client_notification_handler.hpp"
#include "servicefabric/transport_dummy_msg_disposer.hpp"
#include "servicefabric/transport_dummy_server_conn_handler.hpp"
#include "servicefabric/transport_message.hpp"
#include "servicefabric/waitable_callback.hpp"

#include "boost/log/trivial.hpp"

import kvtransport;

namespace sf = servicefabric;

std::wstring resolve_kvstore() { return L""; }

int main() {
  resolve_kvstore();
  // return 0;

  FABRIC_SECURITY_CREDENTIALS cred = {};
  cred.Kind = FABRIC_SECURITY_CREDENTIAL_KIND_NONE;

  FABRIC_TRANSPORT_SETTINGS settings = {};
  settings.KeepAliveTimeoutInSeconds = 10;
  settings.MaxConcurrentCalls = 10;
  settings.MaxMessageSize = 100;
  settings.MaxQueueSize = 100;
  settings.OperationTimeoutInSeconds = 30;
  settings.SecurityCredentials = &cred;

  std::wstring addr_str = L"";

  belt::com::com_ptr<IFabricTransportCallbackMessageHandler> client_notify_h =
      sf::transport_dummy_client_notification_handler::create_instance()
          .to_ptr();

  belt::com::com_ptr<IFabricTransportClientEventHandler> client_event_h =
      sf::transport_dummy_client_conn_handler::create_instance().to_ptr();
  belt::com::com_ptr<IFabricTransportMessageDisposer> client_msg_disposer =
      sf::transport_dummy_msg_disposer::create_instance().to_ptr();
  ;
  belt::com::com_ptr<IFabricTransportClient> client;

  HRESULT hr = S_OK;

  // open client
  hr = CreateFabricTransportClient(
      /* [in] */ IID_IFabricTransportClient,
      /* [in] */ &settings,
      /* [in] */ addr_str.c_str(),
      /* [in] */ client_notify_h.get(),
      /* [in] */ client_event_h.get(),
      /* [in] */ client_msg_disposer.get(),
      /* [retval][out] */ client.put());
  if (hr != S_OK) {
    BOOST_LOG_TRIVIAL(debug) << "CreateFabricTransportClient failed: " << hr;
    return hr;
  }

  // open client
  {
    belt::com::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        sf::FabricAsyncOperationWaitableCallback::create_instance().to_ptr();
    belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
    hr = client->BeginOpen(1000, callback.get(), ctx.put());
    if (hr != S_OK) {
      BOOST_LOG_TRIVIAL(debug) << "BeginOpen failed: " << hr;
      return hr;
    }
    callback->Wait();
    hr = client->EndOpen(ctx.get());
    if (hr != S_OK) {
      BOOST_LOG_TRIVIAL(debug) << "EndOpen failed: " << hr;
      return hr;
    }
  }

  kv_client kvc(client);
}