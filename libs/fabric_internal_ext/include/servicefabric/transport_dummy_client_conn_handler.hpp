// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "fabrictransport_.h"
#include <winrt/base.h>

namespace servicefabric {

class transport_dummy_client_conn_handler
    : public winrt::implements<transport_dummy_client_conn_handler,
                               IFabricTransportClientEventHandler> {
public:
  HRESULT STDMETHODCALLTYPE OnConnected(
      /* [in] */ LPCWSTR connectionAddress) override;

  HRESULT STDMETHODCALLTYPE OnDisconnected(
      /* [in] */ LPCWSTR connectionAddress,
      /* [in] */ HRESULT error) override;
};

} // namespace servicefabric