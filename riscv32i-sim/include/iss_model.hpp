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

  enum priv_level { user = 0, supervisor, reserved, machine };

  priv_level mode = machine;

  reg_file      regfile{};
  sparse_memory mem{};

  uint32_t alu_out;
  uint32_t mem_out;

  uint32_t csrs[4096];

  void exec(decoder_out &dec);
  void exec_alu(decoder_out &dec);
  void exec_alu_branch(decoder_out &dec);
  void mem_phase(decoder_out &dec);
  void wb_retire_phase(decoder_out &dec);
  void wb_retire_ls(decoder_out &dec);
  void wb_retire_alu(decoder_out &dec);
  void csr(decoder_out &dec);

  enum class interrupt_cause {
    SW_U    = 0x0,
    SW_S    = 0x1,
    SW_M    = 0x3,
    Timer_U = 0x4,
    Timer_S = 0x5,
    Timer_M = 0x7,
    Ext_U   = 0x8,
    Ext_S   = 0x9,
    Ext_M   = 0x11
  };

  enum class exception_cause {
    InstAddrMisaligned  = 0x0,
    InstAccFault        = 0x1,
    IllegalInst         = 0x2,
    Breakpoint          = 0x3,
    LocalAddrMisaligned = 0x4,
    LocalAccFault       = 0x5,
    StoreAccMisaligned  = 0x6,
    StoreAccFault       = 0x7,
    ECALL_U             = 0x8,
    ECALL_S             = 0x9,
    ECALL_M             = 0x11,
    InstPageFault       = 0x12,
    LoadPageFault       = 0x13,
    StorePageFault      = 0x15,
  };

  bool terminate = false;

  uint32_t tohost_addr;
  uint32_t PC{0};

  void load(uint32_t virt_addr, void *ptr, int64_t size_in_bytes) {
    mem.write_blocks(virt_addr, ptr, size_in_bytes);
  }

public:
  bool done() const { return terminate; }
  void step();

  void     write_csr(uint32_t addr, uint32_t v);
  uint32_t read_csr(uint32_t addr);

  friend class loader;
};
