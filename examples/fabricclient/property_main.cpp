// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "FabricClient.h"

#include <iostream>
#include <string>

#include "servicefabric/fabric_error.hpp"
#include <servicefabric/waitable_callback.hpp>

#include <argparse/argparse.hpp>
#include <thread>

namespace sf = servicefabric;

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
    argparse::ArgumentParser program("property_main");

    program.add_argument("--uri").required().help("uri");

    program.add_argument("--name").required().help("name");

    program.add_argument("--type").default_value(0).scan<'i', int>().help(
        "type");

    program.parse_args(argc, argv);

    uri = program.get<std::string>("--uri");
    name = program.get<std::string>("--name");
    type = program.get<int>("--type");
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
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

  std::cout << "Calling BeginGetProperty with " << uri << " and " << name
            << std::endl;
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