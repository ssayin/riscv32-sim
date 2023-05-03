// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include "reg_file.hpp"

#include <cassert>

uint32_t reg_file::read(uint8_t index) {
  assert(index < reg_count);
  return x[index];
}

void reg_file::write(uint8_t index, uint32_t data) {
  assert(index < reg_count);
  if (index == 0) return;
  x[index] = data;
}

void reg_file::write(uint8_t index, uint32_t data, gpr_change &out) {
  out.index = index;
  out.next  = data;
  out.prev  = x[index];
  write(index, data);
}

void reg_file::write(uint8_t index, uint32_t data,
                     std::vector<gpr_change> &vec) {
  gpr_change tmp;
  write(index, data, tmp);
  vec.emplace_back(tmp);
}
