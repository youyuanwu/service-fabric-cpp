#pragma once
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

// Implement a timer to run on io ctx on main thread.
// This is a hack to keep the main thread alive.

namespace net = boost::asio;

void timer_loop(net::deadline_timer *timer, const boost::system::error_code &) {
  BOOST_LOG_TRIVIAL(debug) << "timer_loop";

  // Reschedule the timer for 1 second in the future:
  timer->expires_from_now(boost::posix_time::seconds(15));
  // Posts the timer event
  timer->async_wait(std::bind(timer_loop, timer, std::placeholders::_1));
}