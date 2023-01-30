#ifndef RISCV32_SIM_TCP_IP
#define RISCV32_SIM_TCP_IP

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>
#ifdef __unix__
#include <arpa/inet.h>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

template <class Func, class... Args> int call_guard(Func func, Args &&...args) {
  int ret = func(std::forward<Args>(args)...);
  if (ret < 0) throw std::runtime_error(std::strerror(errno));
  return ret;
}

// EWOULDBLOCK => accept(), recv(), recvfrom(), send(), sendmsg(), read() ...
template <class Func, class... Args>
std::optional<int> call_guard_would_block(Func func, Args &&...args) {
  int ret = func(std::forward<Args>(args)...);
  if (ret < 0) {
    if ((errno != EWOULDBLOCK) &&
        /*POSIX.1-2001*/ (errno != EAGAIN)) {
      throw std::runtime_error(std::strerror(errno));
    } else
      return std::nullopt;
  }
  return ret;
}

class fd_factory;
class fd {
  friend class fd_factory;

public:
  fd(const fd &, int x) : handle{x} {}
  fd(const fd &)      = delete;
  fd &operator=(fd &) = delete;
  ~fd();
  explicit operator int() const { return handle; }

private:
  int handle;
  explicit fd(int h) : handle{h} {}

  template <typename... T> static ::std::shared_ptr<fd> create(T &&...args) {
    return ::std::make_shared<fd>(/* FIXME: causes ~fd() to be called */ fd{-1},
                                  ::std::forward<T>(args)...);
  }
};

class fd_factory {
private:
  std::vector<std::shared_ptr<fd>> cont;

public:
  std::shared_ptr<fd> make_fd(int h) {
    return cont.emplace_back(fd::create(h));
  }
};

// using sock_fd = int_handle<decltype(socket), int, int, int>;
// using conn_fd = int_handle<decltype(accept), int, sockaddr *, socklen_t *>;

class irq_server {
public:
  irq_server(int port, int backlog);
  bool poll(int timeout = 6000);
  ~irq_server();

private:
  void                accept();
  std::vector<pollfd> vfd;
  std::shared_ptr<fd> sfd;
  fd_factory          fdf;
};

#endif
