#ifndef INSTR_RV32_ISN_HPP
#define INSTR_RV32_ISN_HPP

#include "common/consts.hpp"
#include "common/offset.hpp"
#include "common/types.hpp"
#include <bitset>
#include <cstdint>
#include <numeric>
#include <utility>

#include <cstdint>

enum class opcode : uint8_t {
  branch   = 0b1100011,
  load     = 0b0000011,
  store    = 0b0100011,
  reg_imm  = 0b0010011,
  reg_reg  = 0b0110011,
  misc_mem = 0b0001111,
  sys      = 0b1110011,
  lui      = 0b0110111,
  auipc    = 0b0010111,
  jal      = 0b1101111,
  jalr     = 0b1100111,
};

enum class branch : uint8_t {
  beq  = 0b000,
  bne  = 0b001,
  blt  = 0b100,
  bge  = 0b101,
  bltu = 0b110,
  bgeu = 0b111
};

enum class load : uint8_t {
  lb  = 0b000,
  lh  = 0b001,
  lw  = 0b010,
  lbu = 0b100,
  lhu = 0b101
};

enum class store : uint8_t {
  sb = 0b000,
  sh = 0b001,
  sw = 0b010,
};

enum class reg_imm : uint8_t {
  addi      = 0b000,
  slti      = 0b010,
  sltiu     = 0b011,
  xori      = 0b100,
  ori       = 0b110,
  andi      = 0b111,
  slli      = 0b001,
  srli_srai = 0b101,
};

enum class reg_reg : uint8_t {
  add_sub_mul  = 0b000,
  sll_mulh     = 0b001,
  slt_mulhsu   = 0b010,
  sltu_mulhu   = 0b011,
  xor_div      = 0b100,
  srl_sra_divu = 0b101,
  or_rem       = 0b110,
  and_remu     = 0b111,
};

enum class misc_mem : uint8_t {
  fence  = 0b000,
  fencei = 0b001,
};

enum class sys : uint8_t {
  other  = 0b000,
  csrrw  = 0b001,
  csrrs  = 0b010,
  csrrc  = 0b011,
  csrrwi = 0b101,
  csrrsi = 0b110,
  csrrci = 0b111,
};

inline uint32_t rv32_imm_i(uint32_t x) { return static_cast<int32_t>(x) >> 20; }

inline uint32_t rv32_imm_s(uint32_t x) {
  return (offset<7U, 11U>(x) | ((static_cast<int32_t>(x & 0xFE000000) >> 20)));
}

inline uint32_t rv32_imm_b(uint32_t x) {
  return ((offset<8U, 11U>(x) << 1) | (offset<25U, 30U>(x) << 5) |
          (offset<7U, 7U>(x) << 11) |
          (static_cast<int32_t>(x & masks::sign_bit) >> 19)) &
         0xFFFFFFFE;
}

inline uint32_t rv32_imm_u(uint32_t x) { return x & masks::type_u_imm; }

inline uint32_t rv32_imm_j(uint32_t x) {
  return (rv32_imm_i(x) & 0xFFF007FE) | (offset<12U, 19U>(x) << 12) |
         (offset<20U, 20U>(x) << 11);
}

struct rv32_jal {
  uint8_t  rd;
  uint32_t imm;
  rv32_jal(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = off::rd(word);
    imm = rv32_imm_j(word);
  }
};

#define RV32_REG_REG_INST(name, funct3, funct7)                                \
  struct rv32_##name {                                                         \
    uint8_t rd;                                                                \
    uint8_t rs1;                                                               \
    uint8_t rs2;                                                               \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rd  = off::rd(word);                                                     \
      rs1 = off::rs1(word);                                                    \
      rs2 = off::rs2(word);                                                    \
    }                                                                          \
  };

RV32_REG_REG_INST(sll, reg_reg::sll_mulh, 0x0)
RV32_REG_REG_INST(srl, reg_reg::srl_sra_divu, 0x0)
RV32_REG_REG_INST(sra, reg_reg::srl_sra_divu, 0x20)

RV32_REG_REG_INST(add, reg_reg::add_sub_mul, 0x0)
RV32_REG_REG_INST(sub, reg_reg::add_sub_mul, 0x20)

RV32_REG_REG_INST(slt, reg_reg::slt_mulhsu, 0x0)
RV32_REG_REG_INST(sltu, reg_reg::sltu_mulhu, 0x0)
RV32_REG_REG_INST(xor, reg_reg::xor_div, 0x0)
RV32_REG_REG_INST(or, reg_reg::or_rem, 0x0)
RV32_REG_REG_INST(and, reg_reg::and_remu, 0x0)

RV32_REG_REG_INST(mulh, reg_reg::sll_mulh, 0x1)
RV32_REG_REG_INST(divu, reg_reg::srl_sra_divu, 0x1)
RV32_REG_REG_INST(mul, reg_reg::add_sub_mul, 0x1)
RV32_REG_REG_INST(mulhsu, reg_reg::slt_mulhsu, 0x1)
RV32_REG_REG_INST(mulhu, reg_reg::sltu_mulhu, 0x1)
RV32_REG_REG_INST(div, reg_reg::xor_div, 0x1)
RV32_REG_REG_INST(rem, reg_reg::or_rem, 0x1)
RV32_REG_REG_INST(remu, reg_reg::and_remu, 0x1)

#undef RV32_REG_REG_INST

#define RV32_REG_IMM_INST(name, funct3, opc)                                   \
  struct rv32_##name {                                                         \
    uint8_t  rd;                                                               \
    uint8_t  rs;                                                               \
    uint32_t imm;                                                              \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rd  = off::rd(word);                                                     \
      rs  = off::rs1(word);                                                    \
      imm = rv32_imm_i(word);                                                  \
    }                                                                          \
  };

RV32_REG_IMM_INST(addi, reg_imm::addi, opcode::reg_imm)
RV32_REG_IMM_INST(xori, reg_imm::xori, opcode::reg_imm)
RV32_REG_IMM_INST(ori, reg_imm::ori, opcode::reg_imm)
RV32_REG_IMM_INST(andi, reg_imm::andi, opcode::reg_imm)
RV32_REG_IMM_INST(slti, reg_imm::slti, opcode::reg_imm)
RV32_REG_IMM_INST(sltiu, reg_imm::sltiu, opcode::reg_imm)

RV32_REG_IMM_INST(lb, load::lb, opcode::load)
RV32_REG_IMM_INST(lh, load::lh, opcode::load)
RV32_REG_IMM_INST(lw, load::lw, opcode::load)
RV32_REG_IMM_INST(lbu, load::lbu, opcode::load)
RV32_REG_IMM_INST(lhu, load::lhu, opcode::load)

RV32_REG_IMM_INST(jalr, 0b000, opcode::jalr)

#undef RV32_REG_IMM_INST

#define RV32_REG_IMM_SH_INST(name, funct3, funct7)                             \
  struct rv32_##name {                                                         \
    uint8_t rd;                                                                \
    uint8_t rs;                                                                \
    uint8_t imm;                                                               \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rd  = off::rd(word);                                                     \
      rs  = off::rs1(word);                                                    \
      imm = off::rs2(word);                                                    \
    }                                                                          \
  };

RV32_REG_IMM_SH_INST(slli, reg_imm::slli, 0x0)
RV32_REG_IMM_SH_INST(srli, reg_imm::srli_srai, 0x0)
RV32_REG_IMM_SH_INST(srai, reg_imm::srli_srai, 0x20)

#undef RV32_REG_IMM_SH_INST

#define RV32_REG_IMM_U_INST(name, opc)                                         \
  struct rv32_##name {                                                         \
    uint8_t  rd;                                                               \
    uint32_t imm;                                                              \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rd  = off::rd(word);                                                     \
      imm = rv32_imm_u(word);                                                  \
    }                                                                          \
  };

RV32_REG_IMM_U_INST(lui, opcode::lui)
RV32_REG_IMM_U_INST(auipc, opcode::auipc)

#undef RV32_REG_IMM_U_INST

#define RV32_STORE_INST(name, funct3)                                          \
  struct rv32_##name {                                                         \
    uint8_t  rs1;                                                              \
    uint8_t  rs2;                                                              \
    uint32_t imm;                                                              \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rs1 = off::rs1(word);                                                    \
      rs2 = off::rs2(word);                                                    \
      imm = rv32_imm_s(word);                                                  \
    }                                                                          \
  };

RV32_STORE_INST(sb, store::sb)
RV32_STORE_INST(sh, store::sh)
RV32_STORE_INST(sw, store::sw)

#undef RV32_STORE_INST

#define RV32_BRANCH_INST(name, funct3)                                         \
  struct rv32_##name {                                                         \
    uint8_t  rs1;                                                              \
    uint8_t  rs2;                                                              \
    uint32_t imm;                                                              \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rs1 = off::rs1(word);                                                    \
      rs2 = off::rs2(word);                                                    \
      imm = rv32_imm_b(word);                                                  \
    }                                                                          \
  };

RV32_BRANCH_INST(beq, branch::beq)
RV32_BRANCH_INST(bne, branch::bne)
RV32_BRANCH_INST(blt, branch::blt)
RV32_BRANCH_INST(bge, branch::bge)
RV32_BRANCH_INST(bltu, branch::bltu)
RV32_BRANCH_INST(bgeu, branch::bgeu)

#undef RV32_BRANCH_INST

#define RV32_CSR_INST(name, funct3)                                            \
  struct rv32_##name {                                                         \
    uint8_t  rd;                                                               \
    uint8_t  rs;                                                               \
    uint32_t csr;                                                              \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rd  = off::rd(word);                                                     \
      rs  = off::rs1(word);                                                    \
      csr = offset<20u, 31u>(word);                                            \
    }                                                                          \
  };

RV32_CSR_INST(csrrw, sys::csrrw)
RV32_CSR_INST(csrrs, sys::csrrs)
RV32_CSR_INST(csrrc, sys::csrrc)
RV32_CSR_INST(csrrwi, sys::csrrwi)
RV32_CSR_INST(csrrsi, sys::csrrsi)
RV32_CSR_INST(csrrci, sys::csrrci)

#undef RV32_CSR_INST

#endif // INSTR_RV32_ISN_HPP
