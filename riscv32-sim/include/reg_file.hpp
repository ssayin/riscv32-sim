#ifndef RISCV32_SIM_REG_FILE_HPP
#define RISCV32_SIM_REG_FILE_HPP

#include <cstdint>
#include <array>

class reg_file {
public:
  uint32_t read(uint8_t index);
  void     write(uint8_t index, uint32_t data);

private:
  std::array<uint32_t, 32> x{};
};

#endif // RISCV32_SIM_REG_FILE_HPP
