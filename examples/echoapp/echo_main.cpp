#include <FabricRuntime.h>
#include <boost/log/trivial.hpp>
#include <moderncom/interfaces.h>
#include <servicefabric/fabric_error.hpp>

#include "loop_timer.hpp"
#include "service_factory.hpp"
#include <boost/asio/signal_set.hpp>

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

  BOOST_LOG_TRIVIAL(debug) << "Wait for debugger to be attached";

  while (!IsDebuggerPresent()) {
    Sleep(1000);
  }
}

HRESULT
get_port(belt::com::ref<IFabricCodePackageActivationContext> activation_context,
         ULONG &port) {
  const FABRIC_ENDPOINT_RESOURCE_DESCRIPTION *echoEndpoint = nullptr;

  HRESULT hr = activation_context->GetServiceEndpointResource(
      L"ServiceEndpoint1", &echoEndpoint);
  if (hr != S_OK) {
    BOOST_LOG_TRIVIAL(error) << "GetServiceEndpointResource failed: " << hr
                             << " " << sf::get_fabric_error_str(hr);
    return hr;
  }
  BOOST_LOG_TRIVIAL(error) << "name: " << echoEndpoint->Name
                           << " port: " << echoEndpoint->Port
                           << " protocol: " << echoEndpoint->Protocol
                           << " type: " << echoEndpoint->Type
                           << " cert: " << echoEndpoint->CertificateName;
  port = echoEndpoint->Port;
  return S_OK;
}

HRESULT get_hostname(
    belt::com::ref<IFabricCodePackageActivationContext> activation_context,
    std::wstring &hostname) {
  // get node ip
  belt::com::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
      sf::FabricAsyncOperationWaitableCallback::create_instance().to_ptr();
  belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
  HRESULT hr = FabricBeginGetNodeContext(1000, callback.get(), ctx.put());
  if (hr != S_OK) {
    BOOST_LOG_TRIVIAL(error)
        << "FabricBeginGetNodeContext failed: " << sf::get_fabric_error_str(hr);
    return hr;
  }
  callback->Wait();
  belt::com::com_ptr<IFabricNodeContextResult> node_ctx;
  hr = FabricEndGetNodeContext(ctx.get(), (void **)node_ctx.put());
  if (hr != S_OK) {
    BOOST_LOG_TRIVIAL(error)
        << "FabricEndGetNodeContext failed: " << sf::get_fabric_error_str(hr);
    return hr;
  }

  const FABRIC_NODE_CONTEXT *node_ctx_res = node_ctx->get_NodeContext();
  if (node_ctx_res == nullptr) {
    BOOST_LOG_TRIVIAL(error) << "FABRIC_NODE_CONTEXT is null";
    return hr;
  }
  BOOST_LOG_TRIVIAL(error) << "Node Ctx info/hostname:"
                           << node_ctx_res->IPAddressOrFQDN;

  hostname = node_ctx_res->IPAddressOrFQDN;
  return S_OK;
}

HRESULT run_app(
    belt::com::ref<IFabricRuntime> fabric_runtime,
    belt::com::ref<IFabricCodePackageActivationContext> activation_context) {

  ULONG port = 0;
  HRESULT hr = get_port(activation_context, port);
  if (hr != S_OK) {
    return hr;
  }
  std::wstring hostname;
  hr = get_hostname(activation_context, hostname);
  if (hr != S_OK) {
    return hr;
  }

  belt::com::com_ptr<IFabricStatelessServiceFactory> service_factory =
      service_factory::create_instance(port, hostname).to_ptr();

  // give runtime onwership
  // Service type name must match from the service manifest
  return fabric_runtime->RegisterStatelessServiceFactory(
      L"EchoAppService", service_factory.detach());
}

int main(int argc, char *argv[]) {
  wait_for_debugger(argc, argv);

  net::io_context io_ctx;

  BOOST_LOG_TRIVIAL(debug) << "App start.";

  belt::com::com_ptr<IFabricRuntime> fabric_runtime;
  belt::com::com_ptr<IFabricCodePackageActivationContext> activation_context;

  HRESULT hr =
      ::FabricCreateRuntime(IID_IFabricRuntime, (void **)fabric_runtime.put());
  if (hr != NO_ERROR) {
    BOOST_LOG_TRIVIAL(error) << "FabricCreateRuntime failed: " << hr << " "
                             << sf::get_fabric_error_str(hr);
    return EXIT_FAILURE;
  }

  hr = ::FabricGetActivationContext(IID_IFabricCodePackageActivationContext,
                                    (void **)activation_context.put());
  if (hr != NO_ERROR) {
    BOOST_LOG_TRIVIAL(error) << "FabricCreateRuntime failed: " << hr;
    return EXIT_FAILURE;
  }

  // app should run forever until kill.
  hr = run_app(fabric_runtime, activation_context);
  if (hr != NO_ERROR) {
    BOOST_LOG_TRIVIAL(error) << "run_app failed: " << hr;
    return EXIT_FAILURE;
  }

  // currently the main thread is not tied with the app and is doing nothing.
  net::deadline_timer timer(io_ctx);
  boost::system::error_code ec;
  net::signal_set signals(io_ctx, SIGINT, SIGTERM);
  signals.async_wait([&io_ctx](auto, auto) {
    BOOST_LOG_TRIVIAL(error) << "Main thread termination signal";
    io_ctx.stop();
  });

  // let main thread loop forever.
  timer_loop(&timer, ec);
  io_ctx.run();
}