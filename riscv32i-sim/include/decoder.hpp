#pragma once

#include <cstdint>
#include <variant>

enum class pipeline_type : uint8_t {
  LS,
  ALU,
  BRANCH,
  CSR,
  TRET,
};

enum class alu_type : uint8_t {
  AND,
  OR,
  XOR,
  ADD,
  SUB,
  SLL,
  SRL,
  SRA,
  SLT,
  SLTU,
  AUIPC,
  JAL,
  JALR,
  MUL,
  MULH,
  MULHSU,
  MULHU,
  DIV,
  DIVU,
  REM,
  REMU
};

enum class ls_type : uint8_t {
  LB,
  LH,
  LW,
  LBU,
  LHU,
  SB,
  SH,
  SW,
};

enum class branch_type : uint8_t {
  BEQ,
  BNE,
  BLT,
  BLTU,
  BGE,
  BGEU,
};

enum class csr_type : uint8_t {
  RW,
  RS,
  RC,
  RWI,
  RSI,
  RCI,
};

enum class trap_ret_type : uint8_t { Machine, Supervisor, User };

using op_type =
    std::variant<alu_type, ls_type, branch_type, csr_type, trap_ret_type>;

struct op {
  uint32_t      imm;
  op_type       opt;
  uint8_t       rd;
  uint8_t       rs1;
  uint8_t       rs2;
  pipeline_type target;
  bool          has_imm;

  constexpr op(bool has_imm, uint8_t rd, uint8_t rs1, uint8_t rs2, op_type opt,
               pipeline_type target, uint32_t imm)
      : has_imm{has_imm}, rd{rd}, rs1{rs1}, rs2{rs2}, opt{opt}, target{target},
        imm{imm} {};
};

op decode(uint32_t word);
