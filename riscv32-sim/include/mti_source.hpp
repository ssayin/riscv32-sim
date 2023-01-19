#ifndef MTI_SOURCE_HPP
#define MTI_SOURCE_HPP

#include "fmt/core.h"
#include "memory/sparse_memory.hpp"
#include <condition_variable>
#include <cstdint>
#include <thread>

class mti_source {
public:
  struct opt {
    uint32_t mtime;
    uint32_t mtimecmp;
    uint32_t interval;
  };

  mti_source(opt opts, sparse_memory &mem);

  bool interrupting() const { return is_interrupting.load(); }

  mti_source(const mti_source &)            = delete;
  mti_source &operator=(mti_source const &) = delete;
  mti_source(mti_source &&)                 = delete;
  mti_source &&operator=(mti_source &&)     = delete;

  ~mti_source();

  void set_exiting(bool exiting = true);

private:
  std::atomic_bool is_interrupting = false;
  std::atomic_bool is_exiting      = false;

  opt            opts;
  sparse_memory &mem;
  std::thread    t;
};

#endif
