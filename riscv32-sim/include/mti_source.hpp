#ifndef MTI_SOURCE_HPP
#define MTI_SOURCE_HPP

#include <array>
#include <atomic>
#include <cstdint>
#include <thread>

class mti_source {
public:
  mti_source(uint32_t interval, std::array<std::atomic<uint8_t>, 8> &mtime);

  mti_source(const mti_source &)            = delete;
  mti_source &operator=(mti_source const &) = delete;
  mti_source(mti_source &&)                 = delete;
  mti_source &&operator=(mti_source &&)     = delete;

  ~mti_source();

  void set_exiting(bool exiting = true);

private:
  std::atomic_bool is_exiting = false;
  std::thread      t;
};

#endif
