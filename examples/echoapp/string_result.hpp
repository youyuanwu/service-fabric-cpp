#pragma once

#include "FabricCommon.h"
#include "FabricRuntime.h"
#include <moderncom/interfaces.h>

class string_result: public belt::com::object<
    string_result,
    IFabricStringResult>{
public:
    string_result(std::wstring str);
    LPCWSTR STDMETHODCALLTYPE get_String() override;
private:
    std::wstring str_;
};