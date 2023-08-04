// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "FabricCommon.h"
#include "fabrictransport_.h"
#include <winrt/base.h>

namespace servicefabric {

// server handle request
class transport_dummy_client_notification_handler
    : public winrt::implements<transport_dummy_client_notification_handler,
                               IFabricTransportCallbackMessageHandler> {
public:
  virtual HRESULT STDMETHODCALLTYPE HandleOneWay(
      /* [in] */ IFabricTransportMessage *message) override;
};

} // namespace servicefabric