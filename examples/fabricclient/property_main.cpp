// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "FabricClient.h"
#include <boost/log/trivial.hpp>

#include <iostream>
#include <string>

#include "servicefabric/fabric_error.hpp"
#include <servicefabric/waitable_callback.hpp>

#include <boost/program_options.hpp>
#include <thread>

namespace sf = servicefabric;
namespace po = boost::program_options;

enum class PPType {
  BINARY, // 0
  INT64,
  DOUBLE,
  WSTRING
};

int main(int argc, char **argv) {
  std::string uri = {};
  std::string name = {};
  int type = {};

  // parse args
  try {

    po::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message")(
        "uri", po::value(&uri)->required(),
        "uri")("name", po::value(&name)->required(),
               "name")("type", po::value(&type)->default_value(0), "type");
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
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what();
    return EXIT_FAILURE;
  }

  std::cout << "pptype is " << std::to_string(static_cast<int>(type))
            << std::endl;
  PPType ptype = static_cast<PPType>(type);

  std::wstring w_uri(uri.begin(), uri.end());
  std::wstring w_name(name.begin(), name.end());

  winrt::com_ptr<IFabricPropertyManagementClient> client;

  HRESULT hr = ::FabricCreateLocalClient(IID_IFabricPropertyManagementClient,
                                         (void **)client.put());

  if (hr != NO_ERROR) {
    std::cout << "client creation failed" << sf::get_fabric_error_str(hr)
              << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "FabricCreateLocalClient success" << std::endl;

  winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
      winrt::make<sf::FabricAsyncOperationWaitableCallback>();

  winrt::com_ptr<IFabricAsyncOperationContext> ctx;

  hr = client->BeginGetProperty(w_uri.c_str(), w_name.c_str(), 1000,
                                callback.get(), ctx.put());
  if (hr != NO_ERROR) {
    std::cout << "BeginGetProperty failed" << sf::get_fabric_error_str(hr)
              << std::endl;
    return EXIT_FAILURE;
  }
  callback->Wait();

  winrt::com_ptr<IFabricPropertyValueResult> result;
  hr = client->EndGetProperty(ctx.get(), result.put());
  if (hr != NO_ERROR) {
    std::cout << "EndGetProperty failed: " << sf::get_fabric_error_str(hr)
              << std::endl;
    return EXIT_FAILURE;
  }

  std::wstring value;
  switch (ptype) {
  case PPType::INT64:
  case PPType::DOUBLE:
    std::cout << "int and double not supported yet" << std::endl;
    break;
  case PPType::WSTRING: {
    LPCWSTR bufferedValue;
    hr = result->GetValueAsWString(&bufferedValue);
    if (hr != NO_ERROR) {
      std::cout << "GetValueAsWString failed:" << sf::get_fabric_error_str(hr)
                << std::endl;
      return EXIT_FAILURE;
    }
    value = std::wstring(bufferedValue);
  } break;
  case PPType::BINARY:
  default: {
    ULONG count;
    const BYTE *data;
    hr = result->GetValueAsBinary(&count, &data);
    if (hr != NO_ERROR) {
      std::cout << "GetValueAsBinary failed:" << sf::get_fabric_error_str(hr)
                << std::endl;
      return EXIT_FAILURE;
    }
    std::string svalue(data, data + count);
    value = std::wstring(svalue.begin(), svalue.end());
  } break;
    break;
  }
  std::wcout << L"Final value: " << value << std::endl;
}