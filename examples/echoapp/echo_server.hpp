#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <utility>

using tcp = boost::asio::ip::tcp;

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
  server(boost::asio::io_context &io_context, short port);

private:
  void do_accept();
  tcp::acceptor acceptor_;
};
