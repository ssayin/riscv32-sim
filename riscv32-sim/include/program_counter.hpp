#ifndef RISCV32_SIM_PROGRAM_COUNTER_HPP
#define RISCV32_SIM_PROGRAM_COUNTER_HPP

#include <cstdint>

class program_counter {
public:
  program_counter(uint32_t x) : pc_next{x + 4}, pc{x} {}
  void set(uint32_t x) { pc_next = x; }
  void update() { pc = pc_next; }

  program_counter(program_counter &&)            = delete;
  program_counter(const program_counter &)       = delete;
  program_counter &operator=(program_counter &&) = delete;
  program_counter &operator=(program_counter &)  = delete;
  ~program_counter()                             = default;

  operator uint32_t() const { return pc; }

private:
  uint32_t pc_next = 0;
  uint32_t pc      = 0;
};

#endif // RISCV32_SIM_PROGRAM_COUNTER_HPP
