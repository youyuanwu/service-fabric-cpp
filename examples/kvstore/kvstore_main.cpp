#include "FabricCommon.h"
#include "FabricRuntime.h"
#include <moderncom/interfaces.h>

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

#include <servicefabric/activation_helpers.hpp>
#include <servicefabric/fabric_error.hpp>

import kvstore;

namespace net = boost::asio;
namespace sf = servicefabric;

void timer_loop(net::deadline_timer *timer, const boost::system::error_code &) {
  BOOST_LOG_TRIVIAL(debug) << "timer_loop";

  // Reschedule the timer for 1 second in the future:
  timer->expires_from_now(boost::posix_time::seconds(15));
  // Posts the timer event
  timer->async_wait(std::bind(timer_loop, timer, std::placeholders::_1));
}

int main(int argc, char *argv[]) {

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

  std::wstring hostname;
  hr = sf::get_hostname(hostname);
  if (hr != NO_ERROR) {
    BOOST_LOG_TRIVIAL(error) << "get_hostname failed: " << hr;
  }

  // app should run forever until kill.
  // hr = run_app(fabric_runtime, activation_context);
  belt::com::com_ptr<IFabricStatefulServiceFactory> service_factory =
      service_factory::create_instance(activation_context, hostname).to_ptr();
  hr = fabric_runtime->RegisterStatefulServiceFactory(L"KvStoreService",
                                                      service_factory.detach());

  if (hr != NO_ERROR) {
    BOOST_LOG_TRIVIAL(error) << "RegisterStatefulServiceFactory failed: " << hr;
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