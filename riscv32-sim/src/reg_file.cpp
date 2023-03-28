// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include "reg_file.hpp"

#include <cassert>
#include <fmt/color.h>
#include <fmt/core.h>

uint32_t reg_file::read(uint8_t index) {
  assert(index < reg_count);
  return x[index];
}

void reg_file::write(uint8_t index, uint32_t data) {
  assert(index < reg_count);
  if (index == 0) return;

  trace(index, x[index], data);
  x[index] = data;
}

void reg_file::trace(uint8_t index, uint32_t prev, uint32_t cur) {
  fmt::print(fg(fmt::color{0xE8EDDF}), "\tx{} ", index);
  fmt::print("old: ");
  print(prev);
  fmt::print("new: ");
  print(cur);
}
void reg_file::print(unsigned int data) {
  fmt::print(fg(fmt::color{0xF5CB5C}), "{:#d}", static_cast<int32_t>(data));
  fmt::print(fg(fmt::color{0xCFDBD5}), " ({:#x})\t", data);
}

void reg_file::write(uint8_t index, uint32_t data, gpr_change &out) {
  out.index = index;
  out.next  = data;
  out.prev  = x[index];
}

void reg_file::write(uint8_t index, uint32_t data,
                     std::vector<gpr_change> &vec) {
  gpr_change tmp;
  write(index, data, tmp);
  vec.emplace_back(tmp);
}
