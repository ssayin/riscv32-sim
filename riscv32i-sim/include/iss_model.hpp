#pragma once

#include "decoder.hpp"
#include "sparse_memory.hpp"

#include <array>
#include <cstdint>

class iss_model {
  class reg_file {
  public:
    uint32_t read(uint8_t index);
    void     write(uint8_t index, uint32_t data);

  private:
    std::array<uint32_t, 32> x{};
  };

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

  bool terminate = false;

public:
  bool     done() { return terminate; }
  uint32_t tohost_addr;
  int32_t  PC{0};
  void     step();
  void     load_program(uint32_t virt_addr, void *ptr, uint32_t size_in_bytes) {
        mem.load_program(virt_addr, ptr, size_in_bytes);
  }
};
