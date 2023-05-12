// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include <string_view>
#include <winerror.h>

namespace servicefabric {

std::string get_fabric_error_str(HRESULT hr);

} // namespace servicefabric