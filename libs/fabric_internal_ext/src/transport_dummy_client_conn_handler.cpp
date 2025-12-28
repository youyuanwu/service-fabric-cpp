// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "servicefabric/transport_dummy_client_conn_handler.hpp"
#include <spdlog/spdlog.h>

namespace servicefabric {

HRESULT STDMETHODCALLTYPE transport_dummy_client_conn_handler::OnConnected(
    /* [in] */ LPCWSTR connectionAddress) {
  UNREFERENCED_PARAMETER(connectionAddress);
  spdlog::debug(L"client_event_handler::OnConnected addr: {}",
                connectionAddress);
  return S_OK;
}

HRESULT STDMETHODCALLTYPE transport_dummy_client_conn_handler::OnDisconnected(
    /* [in] */ LPCWSTR connectionAddress,
    /* [in] */ HRESULT error) {
  UNREFERENCED_PARAMETER(connectionAddress);
  UNREFERENCED_PARAMETER(error);
  spdlog::debug(L"client_event_handler::OnDisconnected addr: {}",
                connectionAddress);
  return S_OK;
}

} // namespace servicefabric