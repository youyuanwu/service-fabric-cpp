// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "servicefabric/transport_dummy_msg_disposer.hpp"
#include <spdlog/spdlog.h>

namespace servicefabric {

void STDMETHODCALLTYPE transport_dummy_msg_disposer::Dispose(
    /* [in] */ ULONG Count,
    /* [size_is][in] */ IFabricTransportMessage **messages) {
  UNREFERENCED_PARAMETER(Count);
  UNREFERENCED_PARAMETER(messages);
  spdlog::debug("msg_disposer::Dispose count {}", Count);
}

} // namespace servicefabric