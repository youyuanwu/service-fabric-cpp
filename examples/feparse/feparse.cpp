#include "servicefabric/fabric_error.hpp"
#include <argparse/argparse.hpp>
#include <iostream>
#include <string>

namespace sf = servicefabric;

int main(int argc, char **argv) {
  std::int64_t input = {};
  std::string hex = {};
  try {
    argparse::ArgumentParser program("feparse");

    program.add_argument("--hr")
        .default_value(static_cast<std::int64_t>(S_OK))
        .scan<'i', std::int64_t>()
        .help("error code value");

    program.add_argument("--hex")
        .default_value(std::string("0x0"))
        .help("error code hex");

    program.parse_args(argc, argv);

    input = program.get<std::int64_t>("--hr");
    hex = program.get<std::string>("--hex");
    // example hr value: -2147017793;
    HRESULT hr = static_cast<HRESULT>(input);
    if (hr == 0) {
      // try parse string
      hr = static_cast<HRESULT>(std::stoll(hex, nullptr, 16)); // hex base
    }
    std::string str = sf::get_fabric_error_str(hr);
    std::cout << "FabricError: " << str << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}