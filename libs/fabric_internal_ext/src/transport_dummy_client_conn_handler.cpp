#include "servicefabric/transport_dummy_client_conn_handler.hpp"

namespace servicefabric {

HRESULT STDMETHODCALLTYPE transport_dummy_client_conn_handler::OnConnected(
    /* [in] */ LPCWSTR connectionAddress) {
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "client_event_handler::OnConnected addr: "
                           << connectionAddress;
#endif
  return S_OK;
}

HRESULT STDMETHODCALLTYPE transport_dummy_client_conn_handler::OnDisconnected(
    /* [in] */ LPCWSTR connectionAddress,
    /* [in] */ HRESULT error) {
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "client_event_handler::OnDisconnected addr: "
                           << connectionAddress;
#endif
  return S_OK;
}

} // namespace servicefabric