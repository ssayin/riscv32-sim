#include "computer.hpp"
#include "Defs.hpp"

uint32_t reg_file::read(uint8_t index) {
  assert(index < 32u);
  return x[index];
}

void reg_file::write(uint8_t index, uint32_t data) {
  assert(index < 32u);
  if (index == 0)
    return;
  x[index] = data;
}
