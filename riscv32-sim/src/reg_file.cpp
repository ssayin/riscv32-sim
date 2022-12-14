#include "reg_file.hpp"
#include "fmt/color.h"
#include "fmt/core.h"
#include <cassert>

uint32_t reg_file::read(uint8_t index) {
  assert(index < 32U);
  return x[index];
}

void reg_file::write(uint8_t index, uint32_t data) {
  assert(index < 32U);
  if (index == 0) return;
  x[index] = data;
  print(index);
}

void reg_file::print(uint8_t index) const {
  auto data = x.at(index);
  fmt::print(fg(fmt::color{0xE8EDDF}), "x{}", index);
  fmt::print(" <= ");
  fmt::print(fg(fmt::color{0xF5CB5C}), "{:#d}", static_cast<int32_t>(data));
  fmt::print(fg(fmt::color{0xCFDBD5}), " ({:#x})\t", data);
}
