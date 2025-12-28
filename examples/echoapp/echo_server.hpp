// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include <asio.hpp>
#include <memory>
#include <utility>

using tcp = asio::ip::tcp;
class session : public std::enable_shared_from_this<session> {
public:
  session(tcp::socket socket);

  void start();

private:
  void do_read();

  void do_write(std::size_t length);

  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};

class server {
public:
  server(asio::io_context &io_context, short port);

private:
  void do_accept();
  tcp::acceptor acceptor_;
};
