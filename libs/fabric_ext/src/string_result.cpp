// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "servicefabric/string_result.hpp"
#include "spdlog/spdlog.h"

namespace servicefabric {

string_result::string_result(std::wstring str) : str_(str) {}

LPCWSTR STDMETHODCALLTYPE string_result::get_String() {
  spdlog::debug(L"string_result::get_String {}", str_);
  return str_.c_str();
}

} // namespace servicefabric