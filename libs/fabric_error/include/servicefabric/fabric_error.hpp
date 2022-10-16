#pragma once

#include <string_view>
#include <winerror.h>

namespace servicefabric {

std::string get_fabric_error_str(HRESULT hr);

} // namespace servicefabric