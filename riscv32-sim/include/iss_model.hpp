#ifndef RISCV32_SIM_ISS_MODEL_HPP
#define RISCV32_SIM_ISS_MODEL_HPP

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

#endif // RISCV32_SIM_ISS_MODEL_HPP
