#ifndef RISCV32_SIM_ISS_MODEL_HPP
#define RISCV32_SIM_ISS_MODEL_HPP

#include "config.hpp"
#include "csr.hpp"
#include "decoder.hpp"
#include "loader.hpp"
#include "reg_file.hpp"
#include "sparse_memory.hpp"
#include "trap_cause.hpp"

#include <array>
#include <cstdint>

class iss_model {
  enum class privilege_level {
    user       = 0b00,
    supervisor = 0b01,
    reserved   = 0b10,
    machine    = 0b11
  };

  sparse_memory              mem;
  reg_file                   rf{};
  std::array<uint32_t, 4096> csrs{};
  uint32_t                   tohost_addr{};
  uint32_t                   PC{0};
  uint32_t                   alu_out{};
  uint32_t                   mem_out{};
  
  privilege_level            mode      = privilege_level::machine;
  bool                       terminate = false;
  bool                       trap      = false;

  void                       exec(op &dec);
  void                       exec_alu(op &dec);
  void                       exec_alu_branch(op &dec);

  void                       mem_phase(op &dec);

  void                       wb_retire_phase(op &dec);
  void                       wb_retire_ls(op &dec);
  void                       wb_retire_alu(op &dec);

  void                       csr(op &dec);

  void                       tret(op &dec);
  void                       handle_mret();
  void                       handle_sret();

  void                       trap_setup(trap_cause cause);

  uint32_t                   read_csr(uint32_t addr);
  void                       write_csr(uint32_t addr, uint32_t v);

  void                       handle_trap();

public:
  bool done() const { return terminate; }
  void step();

  iss_model(loader l, sparse_memory &mem);
};

#endif // RISCV32_SIM_ISS_MODEL_HPP
