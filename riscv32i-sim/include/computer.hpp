#pragma once

#include "Defs.hpp"
#include "rv32_decode.hpp"
#include "sparse_memory.hpp"

#include <array>
#include <cstdint>

class reg_file {
public:
  uint32_t read(uint8_t index);
  void     write(uint8_t index, uint32_t data);

private:
  std::array<uint32_t, 32> x{};
};

class Computer {
  int32_t       PC{0};
  uint32_t      PC_Next{0};
  reg_file      regfile{};
  sparse_memory mem{};

  uint32_t alu_out;
  uint32_t mem_out;

  void exec(decoder_out &dec);
  void exec_alu(decoder_out &dec);
  void exec_alu_branch(decoder_out &dec);
  void mem_phase(decoder_out &dec);
  void wb_retire_phase(decoder_out &dec);
  void wb_retire_ls(decoder_out &dec);
  void wb_retire_alu(decoder_out &dec);
};
