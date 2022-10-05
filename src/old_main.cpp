#include <iostream>

#include "Defs.hpp"

int main() {
  Computer c;

  c.Mem = {0x20, 0x23, 0x1, 0x2F};

  std::cout << std::to_string(c.read_byte(0)) << std::endl;
  std::cout << std::to_string(c.read_byte(1)) << std::endl;
  std::cout << std::to_string(c.read_half(0)) << std::endl;
  std::cout << std::to_string(c.read_word(0)) << std::endl;

  // addi x13,x8,1
  // li x16,31
  // sub x13,x16,x13
  // sra x1,x13,0x3
  // j 0

  c.Mem = {0x93, 0x06, 0x14, 0x00, 0x13, 0x08, 0xf0, 0x01, 0xB3, 0x06,
           0xD8, 0x40, 0x93, 0xD0, 0x36, 0x40, 0x6F, 0xF0, 0x1F, 0xFF};
  c.step();

  std::cout << "PC: " << c.PC << " ";
  std::cout << c.x[13] << std::endl; // prints 1

  c.step();

  std::cout << "PC: " << c.PC << " ";
  std::cout << c.x[16] << std::endl; // prints 31

  c.step();

  std::cout << "PC: " << c.PC << " ";
  std::cout << c.x[13] << std::endl; // 30

  c.step();

  std::cout << "PC: " << c.PC << " ";
  std::cout << c.x[1] << std::endl; // 3

  c.step(); // jump
  std::cout << "PC: " << c.PC << " " << std::endl;
  c.step();

  std::cout << "PC: " << c.PC << " ";
  std::cout << c.x[13] << std::endl;

  c.step();
  std::cout << "PC: " << c.PC << " ";
  std::cout << c.x[16] << std::endl;
  return 0;
}
