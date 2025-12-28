// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "echo_server.hpp"
#include <spdlog/spdlog.h>

using tcp = asio::ip::tcp;

session::session(tcp::socket socket) : socket_(std::move(socket)) {}

void session::start() { do_read(); }

void session::do_read() {
  auto self(shared_from_this());
  socket_.async_read_some(
      asio::buffer(data_, max_length),
      [this, self](asio::error_code ec, std::size_t length) {
        if (!ec) {
          do_write(length);
        }
      });
}

void session::do_write(std::size_t length) {
  auto self(shared_from_this());
  asio::async_write(socket_, asio::buffer(data_, length),
                    [this, self](asio::error_code ec, std::size_t len) {
                      UNREFERENCED_PARAMETER(len);
                      if (!ec) {
                        do_read();
                      }
                    });
}

server::server(asio::io_context &io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
  do_accept();
}

void server::do_accept() {
  acceptor_.async_accept([this](asio::error_code ec, tcp::socket socket) {
    if (!ec) {
      std::make_shared<session>(std::move(socket))->start();
    }

    do_accept();
  });
}