#include "servicefabric/transport_dummy_server_conn_handler.hpp"

#include <servicefabric/async_context.hpp>

namespace servicefabric {

HRESULT STDMETHODCALLTYPE
transport_dummy_server_conn_handler::BeginProcessConnect(
    /* [in] */ IFabricTransportClientConnection *clientConnection,
    /* [in] */ DWORD timeoutMilliseconds,
    /* [in] */ IFabricAsyncOperationCallback *callback,
    /* [retval][out] */ IFabricAsyncOperationContext **context) {
  UNREFERENCED_PARAMETER(clientConnection);
  UNREFERENCED_PARAMETER(timeoutMilliseconds);
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "conn_handler::BeginProcessConnect";
#endif

  belt::com::com_ptr<IFabricAsyncOperationContext> ctx =
      async_context::create_instance(callback).to_ptr();

  *context = ctx.detach();

  return S_OK;
}

HRESULT STDMETHODCALLTYPE
transport_dummy_server_conn_handler::EndProcessConnect(
    /* [in] */ IFabricAsyncOperationContext *context) {
  UNREFERENCED_PARAMETER(context);
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "conn_handler::EndProcessConnect";
#endif
  return S_OK;
}

HRESULT STDMETHODCALLTYPE
transport_dummy_server_conn_handler::BeginProcessDisconnect(
    /* [in] */ COMMUNICATION_CLIENT_ID clientId,
    /* [in] */ DWORD timeoutMilliseconds,
    /* [in] */ IFabricAsyncOperationCallback *callback,
    /* [retval][out] */ IFabricAsyncOperationContext **context) {
  UNREFERENCED_PARAMETER(clientId);
  UNREFERENCED_PARAMETER(timeoutMilliseconds);
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "conn_handler::BeginProcessDisconnect";
#endif
  belt::com::com_ptr<IFabricAsyncOperationContext> ctx =
      async_context::create_instance(callback).to_ptr();
  *context = ctx.detach();
  return S_OK;
}

HRESULT STDMETHODCALLTYPE
transport_dummy_server_conn_handler::EndProcessDisconnect(
    /* [in] */ IFabricAsyncOperationContext *context) {
  UNREFERENCED_PARAMETER(context);
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "conn_handler::EndProcessDisconnect";
#endif
  return S_OK;
}

} // namespace servicefabric