#pragma once

#include "fabrictransport_.h"
#include <moderncom/interfaces.h>

namespace servicefabric {

class transport_dummy_client_conn_handler
    : public belt::com::object<transport_dummy_client_conn_handler,
                               IFabricTransportClientEventHandler> {
public:
  HRESULT STDMETHODCALLTYPE OnConnected(
      /* [in] */ LPCWSTR connectionAddress) override;

  HRESULT STDMETHODCALLTYPE OnDisconnected(
      /* [in] */ LPCWSTR connectionAddress,
      /* [in] */ HRESULT error) override;
};

} // namespace servicefabric