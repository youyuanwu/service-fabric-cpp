#pragma once

#include "FabricCommon.h"
#include "fabrictransport_.h"
#include <moderncom/interfaces.h>

namespace servicefabric {

// server handle request
class transport_dummy_client_notification_handler
    : public belt::com::object<transport_dummy_client_notification_handler,
                               IFabricTransportCallbackMessageHandler> {
public:
  virtual HRESULT STDMETHODCALLTYPE HandleOneWay(
      /* [in] */ IFabricTransportMessage *message) override;
};

} // namespace servicefabric