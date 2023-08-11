#include "servicefabric/fabric_error.hpp"
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

namespace sf = servicefabric;

namespace po = boost::program_options;

int main(int argc, char **argv) {
  std::int64_t input = {};

  try {

    po::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message")(
        "hr", po::value(&input)->default_value(S_OK), "error code value");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::stringstream ss;
      ss << std::endl;
      desc.print(ss);
      std::cerr << ss.str();
      return EXIT_FAILURE;
    }

    // example hr value: -2147017793;
    HRESULT hr = static_cast<HRESULT>(input);
    std::string str = sf::get_fabric_error_str(hr);
    std::cout << "FabricError: " << str;
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}