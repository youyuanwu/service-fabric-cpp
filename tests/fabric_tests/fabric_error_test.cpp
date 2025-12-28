// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include <boost/log/trivial.hpp>

#include <boost/ut.hpp>

#include "FabricTypes.h"
#include "servicefabric/fabric_error.hpp"

namespace sf = servicefabric;

boost::ut::suite errors = [] {
  using namespace boost::ut;

  "test_fabric_error_in_range"_test = [] {
    expect(sf::get_fabric_error_str(FABRIC_E_INVALID_ADDRESS) ==
           "FABRIC_E_INVALID_ADDRESS");
    expect(sf::get_fabric_error_str(FABRIC_E_ENDPOINT_NOT_REFERENCED) ==
           "FABRIC_E_ENDPOINT_NOT_REFERENCED");
  };

  "test_fabric_error_out_of_range"_test = [] {
    expect(sf::get_fabric_error_str(E_POINTER) == "Invalid pointer");
    expect(sf::get_fabric_error_str(E_INVALIDARG) ==
           "The parameter is incorrect.");
    expect(sf::get_fabric_error_str(S_OK) ==
           "The operation completed successfully.");
  };
};

int main() {}