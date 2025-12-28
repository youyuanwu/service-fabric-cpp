// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include <FabricRuntime.h>
#include <servicefabric/fabric_error.hpp>
#include <spdlog/spdlog.h>

#include "loop_timer.hpp"
#include "service_factory.hpp"
#include <boost/asio/signal_set.hpp>

#include "servicefabric/activation_helpers.hpp"
#include "servicefabric/waitable_callback.hpp"

namespace sf = servicefabric;

void wait_for_debugger(int argc, char *argv[]) {
  bool has_wait_for_debugger = false;

  for (int i = 0; i < argc; i++) {
    if (std::string("-wait_for_debugger") == std::string(argv[i])) {
      has_wait_for_debugger = true;
    }
  }

  if (!has_wait_for_debugger) {
    return;
  }

  spdlog::debug("Wait for debugger to be attached");

  while (!IsDebuggerPresent()) {
    Sleep(1000);
  }
}

HRESULT run_app(
    winrt::com_ptr<IFabricRuntime> fabric_runtime,
    winrt::com_ptr<IFabricCodePackageActivationContext> activation_context) {

  ULONG port = 0;
  HRESULT hr = sf::get_port(activation_context, L"ServiceEndpoint1", port);
  if (hr != S_OK) {
    return hr;
  }
  std::wstring hostname;
  hr = sf::get_hostname(hostname);
  if (hr != S_OK) {
    return hr;
  }

  winrt::com_ptr<IFabricStatelessServiceFactory> service_factory_ptr =
      winrt::make<service_factory>(port, hostname);

  // give runtime onwership
  // Service type name must match from the service manifest
  return fabric_runtime->RegisterStatelessServiceFactory(
      L"EchoAppService", service_factory_ptr.detach());
}

int main(int argc, char *argv[]) {
  wait_for_debugger(argc, argv);

  net::io_context io_ctx;

  spdlog::debug("App start.");

  winrt::com_ptr<IFabricRuntime> fabric_runtime;
  winrt::com_ptr<IFabricCodePackageActivationContext> activation_context;

  HRESULT hr =
      ::FabricCreateRuntime(IID_IFabricRuntime, (void **)fabric_runtime.put());
  if (hr != NO_ERROR) {
    spdlog::error("FabricCreateRuntime failed: {} {}", hr,
                  sf::get_fabric_error_str(hr));
    return EXIT_FAILURE;
  }

  hr = ::FabricGetActivationContext(IID_IFabricCodePackageActivationContext,
                                    (void **)activation_context.put());
  if (hr != NO_ERROR) {
    spdlog::error("FabricCreateRuntime failed: {}", hr);
    return EXIT_FAILURE;
  }

  // app should run forever until kill.
  hr = run_app(fabric_runtime, activation_context);
  if (hr != NO_ERROR) {
    spdlog::error("run_app failed: {}", hr);
    return EXIT_FAILURE;
  }

  // currently the main thread is not tied with the app and is doing nothing.
  net::system_timer timer(io_ctx);
  boost::system::error_code ec;
  net::signal_set signals(io_ctx, SIGINT, SIGTERM);
  signals.async_wait([&io_ctx](auto, auto) {
    spdlog::error("Main thread termination signal");
    io_ctx.stop();
  });

  // let main thread loop forever.
  timer_loop(&timer, ec);
  io_ctx.run();
}