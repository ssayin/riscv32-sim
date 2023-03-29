/*
 * The original code can be found at
 * https://www.ibm.com/docs/en/i/7.3?topic=designs-using-poll-instead-select
 */

#include "ipc/irq_server.hpp"
#include "ipc/call_guard.hpp"

#include <algorithm>

bool irq_server::poll(int timeout) {
  if (call_guard(::poll, vfd.data(), vfd.size(), timeout) == 0) {
    return false;
  }

  for (auto &x : vfd) {
    if (x.revents == 0) continue;

    if (x.revents != POLLIN) {
      fprintf(stderr, "revents\n");
      return false;
    }
    if (x.fd == sfd->operator int()) {
      try {
        this->accept();
      } catch (std::runtime_error &e) {
        fprintf(stderr, "%s\n", e.what());
        return true;
      }
    } else {
      std::optional<int> rc{std::nullopt};
      try {
        char buf[128];
        do {
          rc = call_guard_would_block(recv, x.fd, buf, sizeof(buf), 0);

          if (!rc.has_value()) break; /* EWOULDBLOCK or EAGAIN */

          /*
           * TODO: not sure if I need to clean-up if recv returns 0.
           * can I return with false?
           */
          if (rc.value() == 0) throw std::runtime_error("conn closed");
          fprintf(stdout, "received: %s\n", buf);
          /* echo bytes back to the client */
          call_guard_would_block(send, x.fd, buf, rc.value(), 0);

        } while (rc.value() > 0);
      } catch (std::runtime_error &err) {
        /* TODO: remove from fd_factory instead */
        close(x.fd);
        x.fd = -1;
        /* TODO: loop on iterators and move this inside the loop */
        vfd.erase(std::remove_if(this->vfd.begin(), this->vfd.end(),
                                 [](const auto &x) { return x.fd == -1; }),
                  this->vfd.end());
      }
    }
  }

  return true;
}

void irq_server::accept() {
  printf("  Listening socket is readable\n");
  std::optional<int> cli_or_opt{std::nullopt};
  do {
    sockaddr_in cli_addr{};
    socklen_t   clilen{sizeof(cli_addr)};
    cli_or_opt = call_guard_would_block(::accept, sfd->operator int(),
                                        (sockaddr *)&cli_addr, &clilen);

    if (cli_or_opt.has_value()) {
      fdf.make_fd(cli_or_opt.value());
      printf("server: got connection %d from %s port %d\n",
             static_cast<int>(cli_or_opt.value()), inet_ntoa(cli_addr.sin_addr),
             ntohs(cli_addr.sin_port));

      vfd.emplace_back(pollfd{static_cast<int>(cli_or_opt.value()), POLLIN, 0});
    }

  } while (cli_or_opt != std::nullopt);
}

irq_server::~irq_server() {
  for (auto &x : vfd) {
    if (x.fd >= 0) close(x.fd);
  }
}

irq_server::irq_server(int port, int backlog) {
  // EWOULDBLOCK => accept(), recv(), recvfrom(), send(), sendmsg(), read() ...
  sfd = fdf.make_fd(call_guard(socket, AF_INET, SOCK_STREAM, 0));
  int enable{1};

  call_guard(setsockopt, sfd->operator int(), SOL_SOCKET, SO_REUSEADDR,
             (char *)&enable, sizeof(enable));

  call_guard(ioctl, sfd->operator int(), FIONBIO, (char *)&enable);

  sockaddr_in serv_addr{0};
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port        = htons(port);

  call_guard(bind, sfd->operator int(), (struct sockaddr *)&serv_addr,
             sizeof(serv_addr));

  call_guard(listen, sfd->operator int(), backlog);
  // listen socket
  vfd.emplace_back(pollfd{sfd->operator int(), POLLIN, 0});
  printf("irq_server ctor\n");
}

fd::~fd() {
  if (handle >= 0) {
    close(handle);
    handle = -1;
  }
}
