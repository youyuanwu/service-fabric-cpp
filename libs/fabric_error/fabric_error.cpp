#include "servicefabric/fabric_error.hpp"
#include "FabricTypes.h"

#define MAGIC_ENUM_RANGE_MIN 0x80071bbc
#define MAGIC_ENUM_RANGE_MAX 0x80071d4b
#include "magic_enum.hpp"

namespace servicefabric {

std::string_view get_fabric_error_str(HRESULT hr) {
  if (hr < FABRIC_E_FIRST_RESERVED_HRESULT ||
      hr > FABRIC_E_LAST_RESERVED_HRESULT) {
    return "not fabric error";
  }

  auto fabric_code = magic_enum::enum_cast<FABRIC_ERROR_CODE>(hr);
  if (fabric_code.has_value()) {
    return magic_enum::enum_name(fabric_code.value());
  }
  return "enum not found. idl maybe too old";
}

} // namespace servicefabric