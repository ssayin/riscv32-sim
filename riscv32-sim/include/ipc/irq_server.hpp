#ifndef IPC_IRQ_SERVER_HPP
#define IPC_IRQ_SERVER_HPP

#include "ipc/fd.hpp"
#include <sys/poll.h>

class irq_server {
public:
  irq_server(const irq_server &)            = default;
  irq_server(irq_server &&)                 = delete;
  irq_server &operator=(const irq_server &) = default;
  irq_server &operator=(irq_server &&)      = delete;
  irq_server(int port, int backlog);
  bool poll(int timeout = default_timeout);
  ~irq_server();

  constexpr static int default_timeout = 6000;

private:
  void                accept();
  std::vector<pollfd> vfd;
  std::shared_ptr<fd> sfd;
  fd_factory          fdf;
};

#endif /* end of include guard: IPC_IRQ_SERVER_HPP */
