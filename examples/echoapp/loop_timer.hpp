// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once
#include <asio.hpp>
#include <chrono>
#include <spdlog/spdlog.h>

// Implement a timer to run on io ctx on main thread.
// This is a hack to keep the main thread alive.

namespace net = asio;

void timer_loop(net::system_timer *timer, const asio::error_code &) {
  spdlog::debug("timer_loop");

  // Reschedule the timer for 1 second in the future:
  timer->expires_after(std::chrono::seconds(15));
  // Posts the timer event
  timer->async_wait(std::bind(timer_loop, timer, std::placeholders::_1));
}