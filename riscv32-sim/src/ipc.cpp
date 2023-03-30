#include "ipc.hpp"

void ipc::session::start() {
  socket_.async_read_some(
      boost::asio::buffer(data_, max_length),
      [capture0 = shared_from_this()](auto &&PH1, auto &&PH2) {
        capture0->handle_read(std::forward<decltype(PH1)>(PH1),
                              std::forward<decltype(PH2)>(PH2));
      });
}

void ipc::session::handle_read(const boost::system::error_code &error,
                               size_t bytes_transferred) {
  if (!error) {
    boost::asio::async_write(
        socket_, boost::asio::buffer(data_, bytes_transferred),
        [capture0 = shared_from_this()](auto &&PH1, auto &&PH2) {
          capture0->handle_write(std::forward<decltype(PH1)>(PH1));
        });
  }
}

void ipc::session::handle_write(const boost::system::error_code &error) {
  if (!error) {
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        [capture0 = shared_from_this()](auto &&PH1, auto &&PH2) {
          capture0->handle_read(std::forward<decltype(PH1)>(PH1),
                                std::forward<decltype(PH2)>(PH2));
        });
  }
}

ipc::server::server(boost::asio::io_context &io_context, short port,
                    size_t thread_pool_size)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
      thread_pool_size_(thread_pool_size), signals_(io_context) {
  start_accept();
}

void ipc::server::run() {
  std::vector<std::thread> threads;
  for (size_t i = 0; i < thread_pool_size_; ++i) {
    threads.emplace_back([this]() { io_context_.run(); });
  }

  signals_.add(SIGINT);
  signals_.add(SIGTERM);
#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
  signals_.async_wait(
      [this](const boost::system::error_code &, int signal_number) {
        std::cout << "Terminating server..." << std::endl;
        stop();
      });

  for (auto &thread : threads) {
    thread.join();
  }
}

void ipc::server::start_accept() {
  std::shared_ptr<session> new_session = std::make_shared<session>(io_context_);
  acceptor_.async_accept(
      new_session->socket(),
      [this, new_session](const boost::system::error_code &error) {
        if (!error) {
          new_session->start();
        }
        start_accept();
      });
}

void ipc::server::handle_accept(session_ptr                     &new_session,
                                const boost::system::error_code &error) {
  if (!error) {
    new_session->start();
    start_accept();
  }
}
