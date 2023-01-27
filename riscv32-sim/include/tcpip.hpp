#ifndef RISCV32_SIM_TCP_IP
#define RISCV32_SIM_TCP_IP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>
#ifdef __unix__
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

/* errno is POSIX AFAIK. Might move this somewhere else later. */
inline void error(const char *msg) {
  std::perror(msg);
  std::exit(EXIT_FAILURE);
}

/* handle RAII wrapper for exception safety */
template <class Func, class... Args> class int_handle {
private:
  int handle;

public:
  int_handle(Func func, Args &&...args) {
    handle = func(std::forward<Args>(args)...);
    /* TODO: throw */
    if (handle < 0) error("error");
  }

  operator int() { return handle; }

  ~int_handle() { close(handle); }
};

#endif
