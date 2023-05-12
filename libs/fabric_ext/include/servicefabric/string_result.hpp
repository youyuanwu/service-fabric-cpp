// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "FabricCommon.h"
// #include "FabricRuntime.h"
#include <moderncom/interfaces.h>

namespace servicefabric {

class string_result
    : public belt::com::object<string_result, IFabricStringResult> {
public:
  string_result(std::wstring str);
  LPCWSTR STDMETHODCALLTYPE get_String() override;

private:
  std::wstring str_;
};

} // namespace servicefabric