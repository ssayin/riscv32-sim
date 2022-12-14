#ifndef RISCV32_SIM_REG_FILE_HPP
#define RISCV32_SIM_REG_FILE_HPP

#include <array>
#include <cstddef>
#include <cstdint>

class reg_file {
  static constexpr size_t reg_count = 32U;
  using file_type                   = std::array<uint32_t, reg_count>;

public:
  uint32_t read(uint8_t index);
  void     write(uint8_t index, uint32_t data);

private:
  file_type x{};
  void      print(uint8_t index) const;
};

#endif // RISCV32_SIM_REG_FILE_HPP
