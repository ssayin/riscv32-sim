// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef REG_FILE_HPP
#define REG_FILE_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "common/common.hpp"

class reg_file {
  static constexpr size_t reg_count = 32U;
  using file_type                   = std::array<uint32_t, reg_count>;

public:
  uint32_t read(uint8_t index);
  void     write(uint8_t index, uint32_t data);
  void     write(uint8_t index, uint32_t data, gpr_change &out);
  void     write(uint8_t index, uint32_t data, std::vector<gpr_change> &vec);

private:
  file_type   x{};
  static void print(unsigned int data);
};

#endif // REG_FILE_HPP
