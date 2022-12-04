#include "reg_file.hpp"
#include "fmt/core.h"
#include "fmt/color.h"
#include <cassert>

uint32_t reg_file::read(uint8_t index) {
  assert(index < 32u);
  return x[index];
}

void reg_file::write(uint8_t index, uint32_t data) {
  assert(index < 32u);
  if (index == 0)
    return;

  fmt::print("x");
  fmt::print(fg(fmt::color{0xE8EDDF}), "{}", index);
  fmt::print(" <= ");
  fmt::print(fg(fmt::color{0xF5CB5C}),"{:#d}", static_cast<int32_t>(data));
  fmt::print(fg(fmt::color{0xCFDBD5})," ({:#x})\t", data);

  x[index] = data;
}
