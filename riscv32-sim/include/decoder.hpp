#ifndef RISCV32_SIM_DECODER_HPP
#define RISCV32_SIM_DECODER_HPP

#include <cstdint>
#include <variant>

enum class pipeline_target : uint8_t {
  mem,
  alu,
  branch,
  csr,
  tret,
};

enum class alu_type : uint8_t {
  _and,
  _or,
  _xor,
  _add,
  _sub,
  _sll,
  _srl,
  _sra,
  _slt,
  _sltu,
  _mul,
  _mulh,
  _mulhsu,
  _mulhu,
  _div,
  _divu,
  _rem,
  _remu,
  _auipc,
  _jal,
  _jalr,
};

enum class mem_type : uint8_t {
  lb,
  lh,
  lw,
  lbu,
  lhu,
  sb,
  sh,
  sw,
};

enum class branch_type : uint8_t {
  beq,
  bne,
  blt,
  bltu,
  bge,
  bgeu,
};

enum class csr_type : uint8_t {
  csrrw,
  csrrs,
  csrrc,
  csrrwi,
  csrrsi,
  csrrci,
};

enum class trap_ret_type : uint8_t { mret, sret };

using op_type =
    std::variant<alu_type, mem_type, branch_type, csr_type, trap_ret_type>;

struct op {
  uint32_t        imm;
  op_type         opt;
  pipeline_target target;
  uint8_t         rd;
  uint8_t         rs1;
  uint8_t         rs2;
  bool            has_imm;

  constexpr op(bool has_imm, uint8_t rd, uint8_t rs1, uint8_t rs2, op_type opt,
               pipeline_target target, uint32_t imm)
      : has_imm{has_imm}, rd{rd}, rs1{rs1}, rs2{rs2}, opt{opt}, target{target},
        imm{imm} {};
};

op decode(uint32_t word);

#endif // RISCV32_SIM_DECODER_HPP
