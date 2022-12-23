#include "FabricCommon.h"
#include "FabricRuntime.h"
#include <moderncom/interfaces.h>

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

#include <servicefabric/activation_helpers.hpp>
#include <servicefabric/fabric_error.hpp>

#include <boost/program_options.hpp>

import kvstore;

namespace net = boost::asio;
namespace sf = servicefabric;
namespace po = boost::program_options;

void timer_loop(net::deadline_timer *timer, const boost::system::error_code &) {
  BOOST_LOG_TRIVIAL(debug) << "timer_loop";

  // Reschedule the timer for 1 second in the future:
  timer->expires_from_now(boost::posix_time::seconds(15));
  // Posts the timer event
  timer->async_wait(std::bind(timer_loop, timer, std::placeholders::_1));
}

// main logic when running in sf mode
HRESULT sf_main() {
  BOOST_LOG_TRIVIAL(debug) << "App start.";

  belt::com::com_ptr<IFabricRuntime> fabric_runtime;
  belt::com::com_ptr<IFabricCodePackageActivationContext> activation_context;

  HRESULT hr =
      ::FabricCreateRuntime(IID_IFabricRuntime, (void **)fabric_runtime.put());
  if (hr != NO_ERROR) {
    BOOST_LOG_TRIVIAL(error) << "FabricCreateRuntime failed: " << hr << " "
                             << sf::get_fabric_error_str(hr);
    return hr;
  }

  hr = ::FabricGetActivationContext(IID_IFabricCodePackageActivationContext,
                                    (void **)activation_context.put());
  if (hr != NO_ERROR) {
    BOOST_LOG_TRIVIAL(error) << "FabricCreateRuntime failed: " << hr;
    return hr;
  }

  std::wstring hostname;
  hr = sf::get_hostname(hostname);
  if (hr != NO_ERROR) {
    BOOST_LOG_TRIVIAL(error) << "get_hostname failed: " << hr;
  }

  // app should run forever until kill.
  // hr = run_app(fabric_runtime, activation_context);
  std::shared_ptr<resolver> resolver_ptr =
      std::make_shared<sf_resolver>(activation_context);

  belt::com::com_ptr<IFabricStatefulServiceFactory> service_factory =
      service_factory::create_instance(resolver_ptr, hostname).to_ptr();
  hr = fabric_runtime->RegisterStatefulServiceFactory(L"KvStoreService",
                                                      service_factory.detach());

  if (hr != NO_ERROR) {
    BOOST_LOG_TRIVIAL(error) << "RegisterStatefulServiceFactory failed: " << hr;
    return hr;
  }
  return S_OK;
}

// main logic when running outside sf
// TODO the open replica seems to complicated to mock. This is not working yet.
HRESULT
local_main(belt::com::com_ptr<IFabricStatefulServiceFactory> &service_factory,
           belt::com::com_ptr<IFabricStatefulServiceReplica> &replica) {
  std::shared_ptr<resolver> resolver_ptr = std::make_shared<dummy_resolver>();
  service_factory =
      service_factory::create_instance(resolver_ptr, L"localhost").to_ptr();

  HRESULT hr = S_OK;
  FABRIC_PARTITION_ID id = {
      0x9f015816, 0xdc03, 0x4d0d, {0x82, 0x95, 0x11, 0x11}};
  FABRIC_REPLICA_ID rid = 888;
  hr = service_factory->CreateReplica(L"KvStoreService",
                                      L"fabric:/KvStore/KvStoreService", 0,
                                      nullptr, id, rid, replica.put());
  if (hr != S_OK) {
    BOOST_LOG_TRIVIAL(error) << "CreateReplica failed: " << hr;
    return hr;
  }

  // FABRIC_REPLICA_OPEN_MODE mode = FABRIC_REPLICA_OPEN_MODE_NEW;
  // replica->BeginOpen(mode,)

  return S_OK;
}

struct kv_options {
  bool local;
};

int main(int argc, char *argv[]) {

  kv_options options;
  po::options_description desc("Allowed options");
  desc.add_options()("help", "produce help message")(
      "local", po::value(&options.local)->default_value(false),
      "local run outside sf. Not working yet.");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    BOOST_LOG_TRIVIAL(info) << desc;
    return 1;
  }

  net::io_context io_ctx;

  // local use
  belt::com::com_ptr<IFabricStatefulServiceFactory> service_factory;
  belt::com::com_ptr<IFabricStatefulServiceReplica> replica;
  HRESULT hr = S_OK;
  if (options.local) {
    hr = local_main(service_factory, replica);
  } else {
    hr = sf_main();
  }

  if (hr != S_OK) {
    return hr;
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