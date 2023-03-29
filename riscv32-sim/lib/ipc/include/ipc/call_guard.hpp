// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef IPC_CALL_GUARD_HPP
#define IPC_CALL_GUARD_HPP

#include <cstring>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>
#ifdef __unix__
#include <arpa/inet.h>
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
    }
    return std::nullopt;
  }
  return ret;
}

#endif /* end of include guard: IPC_CALL_GUARD_HPP */
