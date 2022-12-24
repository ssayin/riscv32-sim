#ifndef ZICSR_SYNC_EXCEPTION_HPP
#define ZICSR_SYNC_EXCEPTION_HPP

#include "zicsr/trap_cause.hpp"
#include <cstdint>

class sync_exception : public std::exception {
public:
  explicit sync_exception(trap_cause cause) : tc{cause} {}
  [[nodiscard]] const char *what() const noexcept override {
    return str_trap_cause(tc).data();
  }
  trap_cause cause() { return tc; }

private:
  trap_cause tc;
};

#endif // ZICSR_SYNC_EXCEPTION_HPP
