// SPDX-FileCopyrightText: 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef IPC_HPP
#define IPC_HPP

#include <boost/asio.hpp>
#include <csignal>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

namespace ipc {

using boost::asio::ip::tcp;

class session : public std::enable_shared_from_this<session> {
public:
  session(boost::asio::io_context &io_context) : socket_(io_context) {}

  tcp::socket &socket() { return socket_; }

  void start();

  void handle_read(const boost::system::error_code &error,
                   size_t                           bytes_transferred);

  void handle_write(const boost::system::error_code &error);

private:
  tcp::socket socket_;
  enum {
    max_length = 1024
  };
  char data_[max_length]{};
};

typedef std::shared_ptr<session> session_ptr;
class server {
public:
  server(boost::asio::io_context &io_context, short port,
         size_t thread_pool_size);

  void run();

  void stop() { io_context_.stop(); }

private:
  void start_accept();

  void handle_accept(session_ptr                     &new_session,
                     const boost::system::error_code &error);

  boost::asio::io_context &io_context_;
  tcp::acceptor            acceptor_;
  size_t                   thread_pool_size_;
  boost::asio::signal_set  signals_;
};
}; // namespace ipc

#endif /* end of include guard: IPC_HPP */
