#pragma once

#include <fabrictransport_.h>
#include <moderncom/interfaces.h>

namespace servicefabric {

class transport_dummy_server_conn_handler
    : public belt::com::object<transport_dummy_server_conn_handler,
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