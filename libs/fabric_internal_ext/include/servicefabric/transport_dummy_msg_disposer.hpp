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

class transport_dummy_msg_disposer
    : public winrt::implements<transport_dummy_msg_disposer,
                               IFabricTransportMessageDisposer> {
public:
  void STDMETHODCALLTYPE Dispose(
      /* [in] */ ULONG Count,
      /* [size_is][in] */ IFabricTransportMessage **messages) override;
};

} // namespace servicefabric