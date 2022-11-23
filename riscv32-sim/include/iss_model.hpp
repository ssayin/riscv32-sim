#ifndef RISCV32_SIM_ISS_MODEL_HPP
#define RISCV32_SIM_ISS_MODEL_HPP

#include "config.hpp"
#include "csr.hpp"
#include "decoder.hpp"
#include "sparse_memory.hpp"
#include "trap_cause.hpp"

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

    enum class privilege_level {
      user       = 0b00,
      supervisor = 0b01,
      reserved   = 0b10,
      machine    = 0b11
    };

  privilege_level mode = privilege_level::machine;

  uint32_t      alu_out{};
  uint32_t      mem_out{};
  bool trap = false;

  void          exec(op &dec);
  void          exec_alu(op &dec);
  void          exec_alu_branch(op &dec);
  void          mem_phase(op &dec);
  void          wb_retire_phase(op &dec);
  void          wb_retire_ls(op &dec);
  void          wb_retire_alu(op &dec);
  void          csr(op &dec);
  void          tret(op &dec);
  void          handle_mret();
  void          handle_sret();

  void          trap_setup(trap_cause cause);

  bool          terminate = false;

  uint32_t      tohost_addr{};
  uint32_t      PC{0};

  void          load(uint32_t virt_addr, void *ptr, int64_t size_in_bytes) {
    mem.write_blocks(virt_addr, ptr, size_in_bytes);
  }

public:
  bool done() const { return terminate; }
  void step();

  friend class loader;

  std::array<uint32_t, 4096> csrs{};



  uint32_t read_csr(uint32_t addr);
  void     write_csr(uint32_t addr, uint32_t v);

  void handle_trap();

  void write_csr(enum csr addr, uint32_t v) { write_csr(static_cast<uint32_t>(addr), v); }
  uint32_t read_csr(enum csr addr) { return read_csr(static_cast<uint32_t>(addr)); }

  iss_model() {
    write_csr(csr::misa, misa_value);
    write_csr(csr::sstatus, 0b1 << 8);
    write_csr(csr::mstatus, 0b11 << 11);
  }

};

#endif // RISCV32_SIM_ISS_MODEL_HPP
