// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "servicefabric/fabric_error.hpp"
#include "FabricTypes.h"
#include <comdef.h>

#define MAGIC_ENUM_RANGE_MIN 0x80071bbc
#define MAGIC_ENUM_RANGE_MAX 0x80071d4b
#include "magic_enum.hpp"

namespace servicefabric {

std::string get_fabric_error_str(HRESULT hr) {
  if (hr < FABRIC_E_FIRST_RESERVED_HRESULT ||
      hr > FABRIC_E_LAST_RESERVED_HRESULT) {
    // try get the error from com
    _com_error err(hr);
    LPCTSTR errMsg = err.ErrorMessage();
    return std::string(errMsg);
  }

  auto fabric_code = magic_enum::enum_cast<FABRIC_ERROR_CODE>(hr);
  if (fabric_code.has_value()) {
    return std::string(magic_enum::enum_name(fabric_code.value()));
  }
  return "enum not found. idl maybe too old";
}

} // namespace servicefabric