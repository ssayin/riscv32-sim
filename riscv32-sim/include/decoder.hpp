#ifndef RISCV32_SIM_DECODER_HPP
#define RISCV32_SIM_DECODER_HPP

#include <cstdint>
#include <variant>

#include "common/masks.hpp"

enum class pipeline_target : uint8_t {
  mem,
  alu,
  branch,
  csr,
  mret,
  illegal,
  ebreak,
  ecall
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

using op_type =
    std::variant<std::monostate, alu_type, load, store, branch, sys>;

struct op {
  uint32_t        imm;
  op_type         opt;
  pipeline_target target;
  uint8_t         rd;
  uint8_t         rs1;
  uint8_t         rs2;
  bool            has_imm;

  constexpr op(bool has_imm, uint8_t rd, uint8_t rs1, uint8_t rs2, op_type opt,
               pipeline_target target, uint32_t imm, bool is_illegal = false,
               bool is_breakpoint = false, bool is_ecall = false)
      : has_imm{has_imm}, rd{rd}, rs1{rs1}, rs2{rs2}, opt{opt}, target{target},
        imm{imm} {}
};

op decode(uint32_t word);

#endif // RISCV32_SIM_DECODER_HPP
