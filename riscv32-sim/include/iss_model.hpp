#ifndef RISCV32_SIM_ISS_MODEL_HPP
#define RISCV32_SIM_ISS_MODEL_HPP

#include "config.hpp"
#include "csr_file.hpp"
#include "decoder.hpp"
#include "common/csr.hpp"
#include "common/trap_cause.hpp"
#include "loader.hpp"
#include "reg_file.hpp"
#include "sparse_memory.hpp"

#include <array>
#include <cstdint>

class iss_model {
  sparse_memory& mem;
  reg_file      rf{};
  csr_file      cf;

  const uint32_t tohost_addr;

  uint32_t PC{0};
  uint32_t alu_out{};
  uint32_t mem_out{};

  privilege_level mode = privilege_level::machine;

  bool terminate = false;

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

public:
  bool done() const { return terminate; }
  void step();

  iss_model(loader l, sparse_memory &mem);
};

#endif // RISCV32_SIM_ISS_MODEL_HPP
