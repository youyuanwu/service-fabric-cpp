#pragma once

#pragma once

#include "fabrictransport_.h"
#include "servicefabric/async_context.hpp"
#include <moderncom/interfaces.h>

#include "message.hpp"

namespace sf = servicefabric;

// server handle request
class client_notification_handler
    : public belt::com::object<client_notification_handler,
                               IFabricTransportCallbackMessageHandler> {
public:
  virtual HRESULT STDMETHODCALLTYPE HandleOneWay(
      /* [in] */ IFabricTransportMessage *message) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "notification_handler::HandleOneWay";
#endif
    return S_OK;
  }
};