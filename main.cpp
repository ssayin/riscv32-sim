#include <iostream>

#include "Defs.hpp"

int main() {
  Computer c;

  c.Mem = {0x20, 0x23, 0x1, 0x2F};

  std::cout << std::to_string(c.read_byte(0)) << std::endl;
  std::cout << std::to_string(c.read_byte(1)) << std::endl;
  std::cout << std::to_string(c.read_half(0)) << std::endl;
  std::cout << std::to_string(c.read_word(0)) << std::endl;

  return 0;
}
