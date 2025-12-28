// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include <spdlog/spdlog.h>

#include "fabricservicecommunication_.h"
#include "servicefabric/async_context.hpp"
#include <winrt/base.h>

namespace sf = servicefabric;

// for client
class conn_event_handler
    : public winrt::implements<conn_event_handler,
                               IFabricServiceConnectionEventHandler> {
public:
  HRESULT STDMETHODCALLTYPE OnConnected(
      /* [in] */ LPCWSTR connectionAddress) override {
    spdlog::debug(L"conn_event_handler::OnConnected connectionAddress: {}",
                  connectionAddress);

    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE OnDisconnected(
      /* [in] */ LPCWSTR connectionAddress,
      /* [in] */ HRESULT error) override {
    spdlog::debug(
        L"conn_event_handler::OnDisconnected connectionAddress: {} error: {}",
        connectionAddress, error);

    return S_OK;
  }
};