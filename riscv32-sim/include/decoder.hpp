#ifndef RISCV32_SIM_DECODER_HPP
#define RISCV32_SIM_DECODER_HPP

#include <cstdint>
#include <variant>

#include "common/masks.hpp"

enum class target : uint8_t {
  mem,
  alu,
  branch,
  csr,
  mret,
  illegal,
  ebreak,
  ecall
};

enum class alu : uint8_t {
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

using op_type = std::variant<std::monostate, alu, load, store, branch, sys>;

struct op {
  uint32_t imm;
  op_type  opt; // 2 bytes
  target   tgt;
  uint8_t  rd;
  uint8_t  rs1;
  uint8_t  rs2;
  bool     has_imm;
};

op decode(uint32_t word);

#endif // RISCV32_SIM_DECODER_HPP
