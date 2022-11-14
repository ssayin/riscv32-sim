#pragma once

#include "csr_handler.hpp"
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

  csr_handler csrh{};

  uint32_t alu_out{};
  uint32_t mem_out{};

  void exec(op &dec);
  void exec_alu(op &dec);
  void exec_alu_branch(op &dec);
  void mem_phase(op &dec);
  void wb_retire_phase(op &dec);
  void wb_retire_ls(op &dec);
  void wb_retire_alu(op &dec);
  void csr(op &dec);
  void tret(op &dec);
  void handle_mret();
  void handle_sret();

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

  uint32_t tohost_addr{};
  uint32_t PC{0};

  void load(uint32_t virt_addr, void *ptr, int64_t size_in_bytes) {
    mem.write_blocks(virt_addr, ptr, size_in_bytes);
  }

public:
  bool done() const { return terminate; }
  void step();
  friend class loader;
};
