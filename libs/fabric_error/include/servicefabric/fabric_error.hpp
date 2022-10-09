#pragma once

#include <string_view>
#include <winerror.h>

namespace servicefabric {

std::string_view get_fabric_error_str(HRESULT hr);

} // namespace servicefabric