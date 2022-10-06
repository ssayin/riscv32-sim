#pragma once
#include "Defs.hpp"
#include "sparse_memory.hpp"
#include <cstdint>

class RegFile {
public:
  uint32_t read(uint32_t index) {
    assert(index < 32u);
    return x[index];
  }

  void write(uint32_t index, uint32_t data) {
    assert(index < 32u);
    if (index == 0)
      return;
    x[index] = data;
  }

private:
  std::array<uint32_t, 32> x{};
};

class Computer {
  int32_t  PC{0};
  uint32_t PC_Next{0};
  RegFile  regfile{};
  Memory   mem{};

public:
  void step() { exec(mem.read_word(PC)); }

  void exec(uint32_t inst);
  void exec(ALUInst inst);
  void exec(ImmediateInst inst);
  void exec(UImmediateInst inst);
  void exec(UJumpInst inst);
  void exec(BranchInst inst);
  void exec(LoadInst inst);
  void exec(StoreInst inst);
  void exec(FenceInst inst);
  void exec(CSREnvInst inst);
};
