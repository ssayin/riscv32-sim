#pragma once

#include <cstdint>

enum class pipeline_type : uint8_t {
  LS,
  ALU,
  JUMP,
  BRANCH,
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

union op_type {
  alu_type    alu;
  ls_type     ls;
  branch_type branch;
};

struct decoder_out {
  bool          has_imm;
  uint8_t       rd;
  uint8_t       rs1;
  uint8_t       rs2;
  op_type       op;
  pipeline_type target;
  uint32_t      imm;
  decoder_out(bool has_imm, uint8_t rd, uint8_t rs1, uint8_t rs2, alu_type alu,
              pipeline_type target, uint32_t imm)
      : has_imm{has_imm}, rd{rd}, rs1{rs1}, rs2{rs2}, target{target}, imm{imm} {

    op.alu = alu;
  }
  decoder_out(bool has_imm, uint8_t rd, uint8_t rs1, uint8_t rs2,
              branch_type branch, pipeline_type target, uint32_t imm)
      : has_imm{has_imm}, rd{rd}, rs1{rs1}, rs2{rs2}, target{target}, imm{imm} {

    op.branch = branch;
  }
  decoder_out(bool has_imm, uint8_t rd, uint8_t rs1, uint8_t rs2, ls_type ls,
              pipeline_type target, uint32_t imm)
      : has_imm{has_imm}, rd{rd}, rs1{rs1}, rs2{rs2}, target{target}, imm{imm} {

    op.ls = ls;
  }
};

decoder_out decode(uint32_t word);
