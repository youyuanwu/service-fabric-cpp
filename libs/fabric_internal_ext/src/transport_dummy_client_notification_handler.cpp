// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "servicefabric/transport_dummy_client_notification_handler.hpp"

namespace servicefabric {

HRESULT STDMETHODCALLTYPE
transport_dummy_client_notification_handler::HandleOneWay(
    /* [in] */ IFabricTransportMessage *message) {
  UNREFERENCED_PARAMETER(message);
#ifdef SF_DEBUG
  spdlog::debug("notification_handler::HandleOneWay");
#endif
  return S_OK;
}

} // namespace servicefabric