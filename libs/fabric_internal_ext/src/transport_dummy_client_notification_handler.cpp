#include "servicefabric/transport_dummy_client_notification_handler.hpp"

namespace servicefabric {

HRESULT STDMETHODCALLTYPE
transport_dummy_client_notification_handler::HandleOneWay(
    /* [in] */ IFabricTransportMessage *message) {
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "notification_handler::HandleOneWay";
#endif
  return S_OK;
}

} // namespace servicefabric