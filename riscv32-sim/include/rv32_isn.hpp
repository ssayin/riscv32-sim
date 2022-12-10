#ifndef RISCV32_SIM_RV32_ISN_HPP
#define RISCV32_SIM_RV32_ISN_HPP

#include "common/consts.hpp"
#include "common/masks.hpp"
#include "common/types.hpp"
#include "offset.hpp"
#include <bitset>
#include <cstdint>
#include <numeric>
#include <utility>

constexpr auto to_int(Enum auto t) {
  return static_cast<std::underlying_type_t<decltype(t)>>(t);
}

struct masks {
  static constexpr uint32_t sign_bit   = 0x80000000;
  static constexpr uint32_t msb_zero   = 0x7FFFFFFF;
  static constexpr uint32_t tvec_type  = 0x00000002;
  static constexpr uint32_t tvec_base  = 0xFFFFFFFC;
  static constexpr uint32_t type_u_imm = 0xFFFFF000;
};

inline uint32_t rv32_imm_i(uint32_t x) { return static_cast<int32_t>(x) >> 20; }

inline uint32_t rv32_imm_s(uint32_t x) {
  return (offset<7u, 11u>(x) | ((static_cast<int32_t>(x) >> 20) & 0xFFFFFFE0));
}

inline uint32_t rv32_imm_b(uint32_t x) {
  return ((rv32_imm_s(x) & 0xFFFFF7FF) | (offset<7u, 7u>(x) << 4)) & 0xFFFFFFFE;
}

inline uint32_t rv32_imm_u(uint32_t x) { return x & masks::type_u_imm; }

inline uint32_t rv32_imm_j(uint32_t x) {
  return (rv32_imm_i(x) & 0xFFF007FE) | (offset<12u, 19u>(x) << 12) |
         (offset<20u, 20u>(x) << 11);
}

struct rv32_jal {
  uint8_t  rd;
  uint32_t imm;
  rv32_jal(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = RD(word);
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
      rd  = RD(word);                                                          \
      rs1 = RS1(word);                                                         \
      rs2 = RS2(word);                                                         \
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
      rd  = RD(word);                                                          \
      rs  = RS1(word);                                                         \
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
    uint8_t  rd;                                                               \
    uint8_t  rs;                                                               \
    uint32_t imm;                                                              \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rd  = RD(word);                                                          \
      rs  = RS1(word);                                                         \
      imm = RS2(word);                                                         \
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
      rd  = RD(word);                                                          \
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
      rs1 = RS1(word);                                                         \
      rs2 = RS2(word);                                                         \
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
      rs1 = RS1(word);                                                         \
      rs2 = RS2(word);                                                         \
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
      rd  = RD(word);                                                          \
      rs  = RS1(word);                                                         \
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

#endif // RISCV32_SIM_RV32_ISN_HPP
