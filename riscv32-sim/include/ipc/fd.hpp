// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef IPC_FD_HPP
#define IPC_FD_HPP

#include <memory>
#include <vector>

class fd_factory;
class fd {
  friend class fd_factory;

public:
  fd(fd &&)            = delete;
  fd &operator=(fd &&) = delete;
  fd(const fd & /*unused*/, int x) : handle{x} {}
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
  fd_factory() = default;
  fd_factory(const fd_factory &)                    = default;
  fd_factory(fd_factory &&)                         = delete;
  fd_factory         &operator=(const fd_factory &) = default;
  fd_factory         &operator=(fd_factory &&)      = delete;
  std::shared_ptr<fd> make_fd(int h) {
    return cont.emplace_back(fd::create(h));
  }

  ~fd_factory() {
    for (auto &x : cont) {
      printf("~fd_factory() %d\n", x->handle);
    }
  }
};

#endif /* end of include guard: IPC_FD_HPP */
