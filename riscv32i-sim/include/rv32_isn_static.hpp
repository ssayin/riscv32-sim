#pragma once

#include "Defs.hpp"

namespace static_dispatch {

template <ALU funct3, uint8_t funct7 = 0x0> struct rv32_alu_op {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  constexpr rv32_alu_op(uint32_t word)
      : rd(unpack_rd(word)), rs1(unpack_rs1(word)), rs2(unpack_rs2(word)) {}

  constexpr uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(funct3), funct7, rd, rs1, rs2);
  }
  constexpr operator uint32_t() const { return pack(); }
};

typedef rv32_alu_op<ALU::ADD_SUB>       rv32_add;
typedef rv32_alu_op<ALU::ADD_SUB, 0x20> rv32_sub;
typedef rv32_alu_op<ALU::XOR>           rv32_xor;
typedef rv32_alu_op<ALU::SLL>           rv32_sll;
typedef rv32_alu_op<ALU::SLT>           rv32_slt;
typedef rv32_alu_op<ALU::SLTU>          rv32_sltu;
typedef rv32_alu_op<ALU::OR>            rv32_or;
typedef rv32_alu_op<ALU::AND>           rv32_and;
typedef rv32_alu_op<ALU::SRL_SRA>       rv32_srl;
typedef rv32_alu_op<ALU::SRL_SRA, 0x20> rv32_sra;

template <Immediate funct3> struct rv32_imm_op {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  constexpr rv32_imm_op(uint32_t word)
      : rd(unpack_rd(word)), rs(unpack_rs1(word)), imm(unpack_imm_i(word)) {}

  constexpr uint32_t pack() const {
    return pack_imm_op(static_cast<uint8_t>(funct3), rd, rs, imm);
  }
  constexpr operator uint32_t() const { return pack(); }
};

typedef rv32_imm_op<Immediate::ADDI>      rv32_addi;
typedef rv32_imm_op<Immediate::XORI>      rv32_xori;
typedef rv32_imm_op<Immediate::ORI>       rv32_ori;
typedef rv32_imm_op<Immediate::ANDI>      rv32_andi;
typedef rv32_imm_op<Immediate::SLLI>      rv32_slli;
typedef rv32_imm_op<Immediate::SRLI_SRAI> rv32_srli_srai;
typedef rv32_imm_op<Immediate::SLTI>      rv32_slti;
typedef rv32_imm_op<Immediate::SLTIU>     rv32_sltiu;

}; // namespace static_dispatch
