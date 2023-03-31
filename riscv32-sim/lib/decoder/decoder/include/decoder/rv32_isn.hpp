// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef DECODER_RV32_ISN_HPP
#define DECODER_RV32_ISN_HPP

#include "common/offset.hpp"
#include "common/types.hpp"

struct rv32 {
  constexpr static uint8_t funct7(uint32_t w) { return offset(w, 25U, 31U); }
  constexpr static uint8_t funct3(uint32_t w) { return offset(w, 12U, 14U); }
  constexpr static uint8_t rs2(uint32_t w) { return offset(w, 20U, 24U); }
  constexpr static uint8_t rs1(uint32_t w) { return offset(w, 15U, 19U); }
  constexpr static uint8_t opc(uint32_t w) { return offset(w, 0U, 6U); }
  constexpr static uint8_t rd(uint32_t w) { return offset(w, 7U, 11U); }

  constexpr static uint32_t imm_i(uint32_t x) {
    return static_cast<uint32_t>(static_cast<int32_t>(x) >> 20);
  }

  constexpr static uint32_t imm_s(uint32_t x) {
    return (offset(x, 7U, 11U) |
            static_cast<uint32_t>(static_cast<int32_t>(x & 0xFE000000) >> 20));
  }

  constexpr static uint32_t imm_b(uint32_t x) {
    return ((offset(x, 8U, 11U) << 1) | (offset(x, 25U, 30U) << 5) |
            (offset(x, 7U, 7U) << 11) |
            static_cast<uint32_t>(static_cast<int32_t>(x & masks::sign_bit) >>
                                  19)) &
           0xFFFFFFFE;
  }

  constexpr static uint32_t imm_u(uint32_t x) { return x & masks::type_u_imm; }

  constexpr static uint32_t imm_j(uint32_t x) {
    return (imm_i(x) & 0xFFF007FE) | (offset(x, 12U, 19U) << 12) |
           (offset(x, 20U, 20U) << 11);
  }
};

struct rv32_jal {
  uint32_t                          imm;
  uint8_t                           rd;
  static constexpr std::string_view str() { return "jal"; };
  rv32_jal(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = rv32::rd(word);
    imm = rv32::imm_j(word);
  }
};

#define RV32_REG_REG_INST(name, funct3, funct7)                                \
  struct rv32_##name {                                                         \
    uint8_t                           rd;                                      \
    uint8_t                           rs1;                                     \
    uint8_t                           rs2;                                     \
    static constexpr std::string_view str() { return "" #name; };              \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rd  = rv32::rd(word);                                                    \
      rs1 = rv32::rs1(word);                                                   \
      rs2 = rv32::rs2(word);                                                   \
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
    uint32_t                          imm;                                     \
    uint8_t                           rd;                                      \
    uint8_t                           rs;                                      \
    static constexpr std::string_view str() { return "" #name; };              \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rd  = rv32::rd(word);                                                    \
      rs  = rv32::rs1(word);                                                   \
      imm = rv32::imm_i(word);                                                 \
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
    uint8_t                           rd;                                      \
    uint8_t                           rs;                                      \
    uint8_t                           imm;                                     \
    static constexpr std::string_view str() { return "" #name; };              \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rd  = rv32::rd(word);                                                    \
      rs  = rv32::rs1(word);                                                   \
      imm = rv32::rs2(word);                                                   \
    }                                                                          \
  };

RV32_REG_IMM_SH_INST(slli, reg_imm::slli, 0x0)
RV32_REG_IMM_SH_INST(srli, reg_imm::srli_srai, 0x0)
RV32_REG_IMM_SH_INST(srai, reg_imm::srli_srai, 0x20)

#undef RV32_REG_IMM_SH_INST

#define RV32_REG_IMM_U_INST(name, opc)                                         \
  struct rv32_##name {                                                         \
    uint32_t                          imm;                                     \
    uint8_t                           rd;                                      \
    static constexpr std::string_view str() { return "" #name; };              \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rd  = rv32::rd(word);                                                    \
      imm = rv32::imm_u(word);                                                 \
    }                                                                          \
  };

RV32_REG_IMM_U_INST(lui, opcode::lui)
RV32_REG_IMM_U_INST(auipc, opcode::auipc)

#undef RV32_REG_IMM_U_INST

#define RV32_STORE_INST(name, funct3)                                          \
  struct rv32_##name {                                                         \
    uint32_t                          imm;                                     \
    uint8_t                           rs1;                                     \
    uint8_t                           rs2;                                     \
    static constexpr std::string_view str() { return "" #name; };              \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rs1 = rv32::rs1(word);                                                   \
      rs2 = rv32::rs2(word);                                                   \
      imm = rv32::imm_s(word);                                                 \
    }                                                                          \
  };

RV32_STORE_INST(sb, store::sb)
RV32_STORE_INST(sh, store::sh)
RV32_STORE_INST(sw, store::sw)

#undef RV32_STORE_INST

#define RV32_BRANCH_INST(name, funct3)                                         \
  struct rv32_##name {                                                         \
    uint32_t                          imm;                                     \
    uint8_t                           rs1;                                     \
    uint8_t                           rs2;                                     \
    static constexpr std::string_view str() { return "" #name; };              \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rs1 = rv32::rs1(word);                                                   \
      rs2 = rv32::rs2(word);                                                   \
      imm = rv32::imm_b(word);                                                 \
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
    uint32_t                          csr;                                     \
    uint8_t                           rd;                                      \
    uint8_t                           rs;                                      \
    static constexpr std::string_view str() { return "" #name; };              \
    rv32_##name(uint32_t word) { unpack(word); }                               \
    void unpack(uint32_t word) {                                               \
      rd  = rv32::rd(word);                                                    \
      rs  = rv32::rs1(word);                                                   \
      csr = offset(word, 20U, 31U);                                            \
    }                                                                          \
  };

RV32_CSR_INST(csrrw, sys::csrrw)
RV32_CSR_INST(csrrs, sys::csrrs)
RV32_CSR_INST(csrrc, sys::csrrc)
RV32_CSR_INST(csrrwi, sys::csrrwi)
RV32_CSR_INST(csrrsi, sys::csrrsi)
RV32_CSR_INST(csrrci, sys::csrrci)

#undef RV32_CSR_INST

#endif /* end of include guard: DECODER_RV32_ISN_HPP */
