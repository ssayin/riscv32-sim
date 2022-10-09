#pragma once

#include "Defs.hpp"
#include "sparse_memory.hpp"

#include <array>
#include <cstdint>

class RegFile {
public:
  uint32_t read(uint8_t index);
  void     write(uint8_t index, uint32_t data);

private:
  std::array<uint32_t, 32> x{};
};

class Computer {
  int32_t  PC{0};
  uint32_t PC_Next{0};
  RegFile  regfile{};
  Memory   mem{};

public:
  void     issue_wb(uint8_t reg, uint32_t v) {}
  uint32_t read_reg(uint8_t index) { return regfile.read(index); }
};
