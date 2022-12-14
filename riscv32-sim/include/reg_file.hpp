#ifndef RISCV32_SIM_REG_FILE_HPP
#define RISCV32_SIM_REG_FILE_HPP

#include <array>
#include <cstdint>
#include <cstddef>

class reg_file {
  static constexpr size_t reg_count = 32U;

public:
  uint32_t read(uint8_t index);
  void     write(uint8_t index, uint32_t data);

private:
  std::array<uint32_t, reg_count> x{};
};

#endif // RISCV32_SIM_REG_FILE_HPP
