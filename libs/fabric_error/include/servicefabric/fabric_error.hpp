#pragma once

#include <winerror.h>
#include <string_view>

namespace servicefabric{

std::string_view get_fabric_error_str(HRESULT hr);

} // namespace