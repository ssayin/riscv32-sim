#ifndef RISCV32_SIM_ISS_MODEL_HPP
#define RISCV32_SIM_ISS_MODEL_HPP

#include "decoder/decoder.hpp"
#include "loader.hpp"
#include "memory/sparse_memory.hpp"
#include "program_counter.hpp"
#include "reg_file.hpp"
#include "zicsr/csr.hpp"
#include "zicsr/csr_file.hpp"
#include "zicsr/trap_cause.hpp"

#include <array>
#include <cstdint>

class iss_model {
public:
  bool     done() const { return _done; }
  uint32_t tohost() const { return mem.read_word(tohost_addr); }
  void     step();

  iss_model(loader l, sparse_memory &mem);
  void throw_on_sync_trap(target tgt) const;

private:
  program_counter PC;
  reg_file        rf{};
  csr_file        cf;
  sparse_memory  &mem;

  privilege_level mode = privilege_level::machine;

  uint32_t alu_out{};
  uint32_t mem_out{};

  bool           _done = false;
  const uint32_t tohost_addr;

  void exec(op &dec);
  void exec_alu(op &dec);
  void exec_alu_branch(op &dec);

  void mem_phase(op &dec);

  void wb_retire_phase(op &dec);
  void wb_retire_ls(op &dec);
  void wb_retire_alu(op &dec);

  void csr(op &dec);
  void trap_setup(trap_cause cause);

  void tret(op &dec);
  void handle_mret();
  void handle_sret();
};

#endif // RISCV32_SIM_ISS_MODEL_HPP
