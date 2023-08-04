// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include <fabrictransport_.h>
#include <winrt/base.h>

namespace servicefabric {

class transport_dummy_server_conn_handler
    : public winrt::implements<transport_dummy_server_conn_handler,
                               IFabricTransportConnectionHandler> {
public:
  HRESULT STDMETHODCALLTYPE BeginProcessConnect(
      /* [in] */ IFabricTransportClientConnection *clientConnection,
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override;

  HRESULT STDMETHODCALLTYPE EndProcessConnect(
      /* [in] */ IFabricAsyncOperationContext *context) override;

  HRESULT STDMETHODCALLTYPE BeginProcessDisconnect(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override;

  HRESULT STDMETHODCALLTYPE EndProcessDisconnect(
      /* [in] */ IFabricAsyncOperationContext *context) override;
};

} // namespace servicefabric