// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#define BOOST_TEST_MODULE fabric_tests

#include <boost/log/trivial.hpp>

#include <boost/test/unit_test.hpp>

#include "FabricTypes.h"
#include "servicefabric/fabric_error.hpp"

namespace sf = servicefabric;

BOOST_AUTO_TEST_SUITE(test_fabric_error)

BOOST_AUTO_TEST_CASE(test_fabric_error_in_range) {
  BOOST_CHECK_EQUAL("FABRIC_E_INVALID_ADDRESS",
                    sf::get_fabric_error_str(FABRIC_E_INVALID_ADDRESS));
  BOOST_CHECK_EQUAL("FABRIC_E_ENDPOINT_NOT_REFERENCED",
                    sf::get_fabric_error_str(FABRIC_E_ENDPOINT_NOT_REFERENCED));
}

BOOST_AUTO_TEST_CASE(test_fabric_error_out_of_range) {
  BOOST_CHECK_EQUAL("Invalid pointer", sf::get_fabric_error_str(E_POINTER));
  BOOST_CHECK_EQUAL("The parameter is incorrect.",
                    sf::get_fabric_error_str(E_INVALIDARG));
  BOOST_CHECK_EQUAL("The operation completed successfully.",
                    sf::get_fabric_error_str(S_OK));
}

BOOST_AUTO_TEST_SUITE_END()