#ifndef RISCV32_SIM_RV32_ISN_HPP
#define RISCV32_SIM_RV32_ISN_HPP

#include "consts.hpp"
#include "masks.hpp"
#include "offset.hpp"
#include "types.hpp"
#include <bitset>
#include <cstdint>
#include <numeric>
#include <utility>

template <Enum T> constexpr std::underlying_type_t<T> to_int(T t) {
  return static_cast<std::underlying_type_t<T>>(t);
}

constexpr auto sign_extend(Integral auto x, Unsigned auto shamt) {
  return static_cast<decltype(x)>(
      static_cast<std::make_signed_t<decltype(x)>>(x) >> shamt);
}

constexpr uint32_t sign_extend(uint32_t in, uint8_t shamt) {
  return static_cast<uint32_t>(static_cast<int32_t>(in << shamt) >> shamt);
}

constexpr uint32_t pack_alu(uint8_t funct3, uint8_t funct7, uint8_t rd,
                            uint8_t rs1, uint8_t rs2) {
  return rd << 7 | funct3 << 12 | rs1 << 15 | rs2 << 20 | funct7 << 25 |
         to_int(opcode::reg_reg);
}

constexpr uint32_t unpack_imm_i(uint32_t word) {
  return offset<20u, 20u>(word) | (offset<21u, 30u>(word) << 1) |
         sign_extend(word & consts::sign_bit_mask, 20u);
}

constexpr uint32_t pack_i_op(uint8_t funct3, uint8_t rd, uint8_t rs,
                             uint32_t imm, opcode opc) {
  return rd << 7 | funct3 << 12 | rs << 15 | imm << 20 | to_int(opc);
}

constexpr uint32_t unpack_imm_u(uint32_t word) {
  return offset<12u, 30u>(word) << 12;
}

constexpr uint32_t pack_imm_u(uint8_t rd, uint32_t imm, uint8_t opc) {
  return opc | rd << 7 | imm;
}

constexpr uint32_t pack_jump_op(uint8_t rd, uint32_t imm, uint8_t opc) {
  return rd << 7 | offset<12u, 19u>(imm) << 12 | offset<11u, 11u>(imm) << 20 |
         offset<1u, 10u>(imm) << 21 | offset<20u, 20u>(imm) << 30 | opc;
}

constexpr uint32_t unpack_imm_j(uint32_t word) {
  return (offset<21u, 30u>(word) << 1) | (offset<20u, 20u>(word) << 11) |
         (offset<12u, 19u>(word) << 12) |
         sign_extend(word & consts::sign_bit_mask, 11u);
}

constexpr uint32_t pack_branch_op(uint8_t funct3, uint8_t rs1, uint8_t rs2,
                                  uint32_t imm) {
  return offset<11u, 11u>(imm) << 7 | offset<1u, 4u>(imm) << 8 | funct3 << 12 |
         rs1 << 15 | rs2 << 20 | offset<5u, 11u>(imm) << 25 |
         offset<12u, 12u>(imm) << 31 | to_int(opcode::branch);
}

constexpr uint32_t unpack_imm_b(uint32_t word) {
  return (offset<8u, 11u>(word) << 1) | (offset<25u, 30u>(word) << 5) |
         (offset<7u, 7u>(word) << 11) |
         sign_extend(word & consts::sign_bit_mask, 19u);
}

constexpr uint32_t pack_store_op(uint8_t funct3, uint8_t rs1, uint8_t rs2,
                                 uint32_t imm) {
  return offset<0u, 4u>(imm) << 7 | funct3 << 12 | rs1 << 15 | rs2 << 20 |
         offset<5u, 11u>(imm) << 25 | to_int(opcode::store);
};

constexpr uint32_t unpack_imm_s(uint32_t word) {
  return RD(word) | (offset<25u, 30u>(word) << 5) |
         sign_extend(word & consts::sign_bit_mask, 20u);
}
constexpr uint32_t unpack_csr(uint32_t word) { return offset<20u, 31u>(word); }

struct rv32_isn {
  virtual void     unpack(uint32_t word) = 0;
  virtual uint32_t pack() const          = 0;
  virtual ~rv32_isn()                    = default;
};

struct rv32_jal : public rv32_isn {
  uint8_t  rd;
  uint32_t imm;
  explicit(false) rv32_jal(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) final {
    rd  = RD(word);
    imm = unpack_imm_j(word);
  }
  uint32_t pack() const final {
    return pack_jump_op(rd, imm, to_int(opcode::jal));
  }
  explicit(false) operator uint32_t() const { return pack(); }
};

#define RV32_REG_REG_INST(name, funct3, funct7)                                \
  struct rv32_##name : public rv32_isn {                                       \
    uint8_t rd;                                                                \
    uint8_t rs1;                                                               \
    uint8_t rs2;                                                               \
    explicit(false) rv32_##name(uint32_t word) { unpack(word); }               \
    void unpack(uint32_t word) final {                                         \
      rd  = RD(word);                                                          \
      rs1 = RS1(word);                                                         \
      rs2 = RS2(word);                                                         \
    }                                                                          \
    uint32_t pack() const final {                                              \
      return pack_alu(static_cast<uint8_t>(funct3), funct7, rd, rs1, rs2);     \
    }                                                                          \
    explicit(false) operator uint32_t() const { return pack(); }               \
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
  struct rv32_##name : public rv32_isn {                                       \
    uint8_t  rd;                                                               \
    uint8_t  rs;                                                               \
    uint32_t imm;                                                              \
    explicit(false) rv32_##name(uint32_t word) { unpack(word); }               \
    void unpack(uint32_t word) final {                                         \
      rd  = RD(word);                                                          \
      rs  = RS1(word);                                                         \
      imm = unpack_imm_i(word);                                                \
    }                                                                          \
    uint32_t pack() const final {                                              \
      return pack_i_op(static_cast<uint8_t>(funct3), rd, rs, imm, opc);        \
    }                                                                          \
    explicit(false) operator uint32_t() const { return pack(); }               \
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
  struct rv32_##name : public rv32_isn {                                       \
    uint8_t  rd;                                                               \
    uint8_t  rs;                                                               \
    uint32_t imm;                                                              \
    explicit(false) rv32_##name(uint32_t word) { unpack(word); }               \
    void unpack(uint32_t word) final {                                         \
      rd  = RD(word);                                                          \
      rs  = RS1(word);                                                         \
      imm = RS2(word);                                                         \
    }                                                                          \
    uint32_t pack() const final {                                              \
      return pack_i_op(static_cast<uint8_t>(funct3), rd, rs, imm,              \
                       opcode::reg_imm) |                                      \
             (funct7 << 25);                                                   \
    }                                                                          \
    explicit(false) operator uint32_t() const { return pack(); }               \
  };

RV32_REG_IMM_SH_INST(slli, reg_imm::slli, 0x0)
RV32_REG_IMM_SH_INST(srli, reg_imm::srli_srai, 0x0)
RV32_REG_IMM_SH_INST(srai, reg_imm::srli_srai, 0x20)

#undef RV32_REG_IMM_SH_INST

#define RV32_REG_IMM_U_INST(name, opc)                                         \
  struct rv32_##name : public rv32_isn {                                       \
    uint8_t  rd;                                                               \
    uint32_t imm;                                                              \
    explicit(false) rv32_##name(uint32_t word) { unpack(word); }               \
    void unpack(uint32_t word) final {                                         \
      rd  = RD(word);                                                          \
      imm = unpack_imm_u(word);                                                \
    }                                                                          \
    uint32_t pack() const final {                                              \
      return pack_imm_u(rd, imm, static_cast<uint8_t>(opc));                   \
    }                                                                          \
    explicit(false) operator uint32_t() const { return pack(); }               \
  };

RV32_REG_IMM_U_INST(lui, opcode::lui)
RV32_REG_IMM_U_INST(auipc, opcode::auipc)

#undef RV32_REG_IMM_U_INST

#define RV32_STORE_INST(name, funct3)                                          \
  struct rv32_##name : public rv32_isn {                                       \
    uint8_t  rs1;                                                              \
    uint8_t  rs2;                                                              \
    uint32_t imm;                                                              \
    explicit(false) rv32_##name(uint32_t word) { unpack(word); }               \
    void unpack(uint32_t word) final {                                         \
      rs1 = RS1(word);                                                         \
      rs2 = RS2(word);                                                         \
      imm = unpack_imm_s(word);                                                \
    }                                                                          \
    uint32_t pack() const final {                                              \
      return pack_store_op(to_int(funct3), rs1, rs2, imm);                     \
    }                                                                          \
    explicit(false) operator uint32_t() const { return pack(); }               \
  };

RV32_STORE_INST(sb, store::sb)
RV32_STORE_INST(sh, store::sh)
RV32_STORE_INST(sw, store::sw)

#undef RV32_STORE_INST

#define RV32_BRANCH_INST(name, funct3)                                         \
  struct rv32_##name : public rv32_isn {                                       \
    uint8_t  rs1;                                                              \
    uint8_t  rs2;                                                              \
    uint32_t imm;                                                              \
    explicit(false) rv32_##name(uint32_t word) { unpack(word); }               \
    void unpack(uint32_t word) final {                                         \
      rs1 = RS1(word);                                                         \
      rs2 = RS2(word);                                                         \
      imm = unpack_imm_b(word);                                                \
    }                                                                          \
    uint32_t pack() const final {                                              \
      return pack_branch_op(to_int(funct3), rs1, rs2, imm);                    \
    }                                                                          \
    explicit(false) operator uint32_t() const { return pack(); }               \
  };

RV32_BRANCH_INST(beq, branch::beq)
RV32_BRANCH_INST(bne, branch::bne)
RV32_BRANCH_INST(blt, branch::blt)
RV32_BRANCH_INST(bge, branch::bge)
RV32_BRANCH_INST(bltu, branch::bltu)
RV32_BRANCH_INST(bgeu, branch::bgeu)

#undef RV32_BRANCH_INST

#define RV32_CSR_INST(name, funct3)                                            \
  struct rv32_##name : public rv32_isn {                                       \
    uint8_t  rd;                                                               \
    uint8_t  rs;                                                               \
    uint32_t csr;                                                              \
    explicit(false) rv32_##name(uint32_t word) { unpack(word); }               \
    void unpack(uint32_t word) final {                                         \
      rd  = RD(word);                                                          \
      rs  = RS1(word);                                                         \
      csr = unpack_csr(word);                                                  \
    }                                                                          \
    uint32_t pack() const final {                                              \
      return rd << 7 | to_int(funct3) << 12 | rs << 15 | csr << 20 |           \
             to_int(opcode::sys);                                              \
    }                                                                          \
    explicit(false) operator uint32_t() const { return pack(); }               \
  };

RV32_CSR_INST(csrrw, sys::csrrw)
RV32_CSR_INST(csrrs, sys::csrrs)
RV32_CSR_INST(csrrc, sys::csrrc)
RV32_CSR_INST(csrrwi, sys::csrrwi)
RV32_CSR_INST(csrrsi, sys::csrrsi)
RV32_CSR_INST(csrrci, sys::csrrci)

#undef RV32_CSR_INST

#define RV32_TRAP_ENV_INST(name, rs2, funct7)                                  \
  struct rv32_##name : public rv32_isn {                                       \
    explicit(false) rv32_##name(uint32_t word) { unpack(word); }               \
    void     unpack(uint32_t word) final {}                                    \
    uint32_t pack() const final {                                              \
      return 0x0 << 7 | 0x0 << 12 | 0x0 << 15 | to_int(rs2) << 20 |            \
             funct7 << 25 | to_int(opcode::sys);                               \
    }                                                                          \
    explicit(false) operator uint32_t() const { return pack(); }               \
  };

RV32_TRAP_ENV_INST(ecall, other_sys::ecall, 0x0)
RV32_TRAP_ENV_INST(ebreak, other_sys::ebreak, 0x0)
RV32_TRAP_ENV_INST(sret, other_sys::trap_ret, 0x8)
RV32_TRAP_ENV_INST(mret, other_sys::trap_ret, 0x24)
RV32_TRAP_ENV_INST(wfi, other_sys::interrupt_management, 0x8)

#undef RV32_TRAP_ENV_INST

#endif // RISCV32_SIM_RV32_ISN_HPP
