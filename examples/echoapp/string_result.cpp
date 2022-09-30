#include <boost/log/trivial.hpp>
#include "string_result.hpp"

string_result::string_result(std::wstring str) : str_(str) {}

LPCWSTR STDMETHODCALLTYPE string_result::get_String(){
    BOOST_LOG_TRIVIAL(debug) << "string_result::get_String " << str_;
    return str_.c_str();
}