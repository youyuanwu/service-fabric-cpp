// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "FabricRuntime.h"
#include <moderncom/interfaces.h>

namespace servicefabric {

// get hostname of the current machine
HRESULT get_hostname(std::wstring &hostname);

// get the port of the endpoint assigned by sf
HRESULT
get_port(belt::com::ref<IFabricCodePackageActivationContext> activation_context,
         std::wstring const &endpoint_name, ULONG &port);

} // namespace servicefabric