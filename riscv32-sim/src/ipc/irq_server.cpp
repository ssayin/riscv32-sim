/*
 * The original code can be found at
 * https://www.ibm.com/docs/en/i/7.3?topic=designs-using-poll-instead-select
 */

#include "ipc/irq_server.hpp"
#include "ipc/call_guard.hpp"

#include <algorithm>
#include <cstring>
#include <optional>
#include <stdexcept>

constexpr static int bufsize = 128;

void irq_server::close(int &fd) {
  ::close(fd);
  fd = -1;
}

void irq_server::accept_all() {
  std::optional<int> opt;
  while ((opt = this->accept()) != std::nullopt) {
    if (opt.has_value()) {
      fdf.make_fd(opt.value());
      vfd.emplace_back(pollfd{static_cast<int>(opt.value()), POLLIN, 0});
    }
  }
}

bool irq_server::for_each_pollfd(pollfd_iterator        first,
                                 const pollfd_iterator &last) {
  for (; first < last; ++first) {

    if (first->revents == 0) continue;

    if (first->revents != POLLIN) {
      fprintf(stderr, "revents\n");
      return false;
    }

    if (first->fd == sfd->operator int()) {
      try {
        accept_all();
      } catch (std::runtime_error &e) {
        fprintf(stderr, "%s\n", e.what());
        return true;
      }

    } else {
      std::optional<int> rc{std::nullopt};
      try {
        char buf[bufsize];
        do {
          rc = call_guard_would_block(recv, first->fd, buf, sizeof(buf), 0);

          if (!rc.has_value()) break; /* EWOULDBLOCK or EAGAIN */
          if (rc.value() == 0) {
            vfd.erase(std::next(first).base());
            printf("%d\n", first->fd);
            ::close(first->fd);
            first->fd = -1;
            printf("%d\n", first->fd);
            // close(first->fd);
            break;
          }
          fprintf(stdout, "received: %s\n", buf);
          /* echo bytes back to the client */
          call_guard_would_block(send, first->fd, buf, rc.value(), 0);

        } while (rc.value() > 0);
      } catch (std::runtime_error &err) {
        printf("%s", err.what());
        vfd.erase(std::next(first).base());
        printf("%d\n", first->fd);
        ::close(first->fd);
        first->fd = -1;
        printf("%d\n", first->fd);
      }
    }
  }

  return true;
}

bool irq_server::poll(int timeout) {
  if (call_guard(::poll, vfd.data(), vfd.size(), timeout) == 0) return false;
  return for_each_pollfd(vfd.rbegin(), vfd.rend());
}

std::optional<int> irq_server::accept() {
  sockaddr_in cli_addr{};
  socklen_t   clilen{sizeof(cli_addr)};
  return call_guard_would_block(::accept, sfd->operator int(),
                                std::bit_cast<struct sockaddr *>(&cli_addr),
                                &clilen);
  /*printf("server: got connection %d from %s port %d\n",
               static_cast<int>(cli_or_opt.value()),
     inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
               */
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
  if (-1 == setsockopt(sfd->operator int(), SOL_SOCKET, SO_REUSEADDR, &enable,
                       sizeof(enable)) ||
      -1 == ioctl(sfd->operator int(), FIONBIO, &enable)) {
    throw std::runtime_error(std::strerror(errno));
  }

  sockaddr_in serv_addr{};
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port        = htons(port);

  call_guard(bind, sfd->operator int(),
             std::bit_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr));
  call_guard(listen, sfd->operator int(), backlog);

  vfd.emplace_back(pollfd{sfd->operator int(), POLLIN, 0});
}

fd::~fd() {
  if (handle >= 0) {
    close(handle);
    handle = -1;
  }
}
