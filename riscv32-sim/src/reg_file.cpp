#include "reg_file.hpp"
#include "fmt/core.h"
#include <cassert>

uint32_t reg_file::read(uint8_t index) {
  assert(index < 32u);
  return x[index];
}

void reg_file::write(uint8_t index, uint32_t data) {
  assert(index < 32u);
  if (index == 0)
    return;
  fmt::print("x{} <= {:#x}\n", index, data);
  x[index] = data;
}
