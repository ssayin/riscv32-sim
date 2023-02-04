#ifndef REG_FILE_HPP
#define REG_FILE_HPP

#include <array>
#include <cstddef>
#include <cstdint>

class reg_file {
  static constexpr size_t reg_count = 32U;
  using file_type                   = std::array<uint32_t, reg_count>;

public:
  uint32_t    read(uint8_t index);
  void        write(uint8_t index, uint32_t data);
  static void trace(uint8_t index, uint32_t prev, uint32_t cur);

private:
  file_type   x{};
  static void print(unsigned int data);
};

#endif // REG_FILE_HPP
