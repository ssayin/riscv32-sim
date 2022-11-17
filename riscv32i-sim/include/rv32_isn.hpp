#pragma once

#include <bitset>
#include <cstdint>
#include <numeric>
#include <type_traits>
#include <utility>

enum class opcode : uint8_t {
  branch    = 0b1100011,
  load      = 0b0000011,
  store     = 0b0100011,
  reg_imm = 0b0010011,
  reg_reg = 0b0110011,
  misc_mem = 0b0001111,
  sys       = 0b1110011,
  lui       = 0b0110111,
  auipc     = 0b0010111,
  jal       = 0b1101111,
  jalr      = 0b1100111,
};

// B-Type
enum class branch : uint8_t {
  beq  = 0b000,
  bne  = 0b001,
  blt  = 0b100,
  bge  = 0b101,
  bltu = 0b110,
  bgeu = 0b111
};

// I-type
enum class load : uint8_t {
  lb  = 0b000,
  lh  = 0b001,
  lw  = 0b010,
  lbu = 0b100,
  lhu = 0b101
};

// S-type
enum class store : uint8_t {
  sb = 0b000,
  sh = 0b001,
  sw = 0b010,
};

// I-type
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

// R-type
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

// I-type
enum class misc_mem : uint8_t {
  fence  = 0b000,
  fencei = 0b001,
};

// I-type
enum class sys : uint8_t {
  other  = 0b000,
  csrrw  = 0b001,
  csrrs  = 0b010,
  csrrc  = 0b011,
  csrrwi = 0b101,
  csrrsi = 0b110,
  csrrci = 0b111,
};

enum class other_sys : uint8_t {
  ecall               = 0x0,
  ebreak              = 0x1,
  trap_ret            = 0x2,
  interrupt_management = 0x5
};

template <typename T>
concept Integral = std::is_integral<T>::value;
template <class T>
concept Unsigned = std::is_unsigned<T>::value;

consteval auto fillbits(Unsigned auto bitcount) {
  if (bitcount == 0u)
    static_assert("fillbits: number of bits cannot be 0");
  auto sum = 1u;
  while (--bitcount) {
    sum = (sum << 1u) | 1u;
  }
  return sum;
}

template <Unsigned auto l, Unsigned auto h>
using diff = std::integral_constant<decltype(h - l + 1u), h - l + 1u>;

template <Unsigned auto l, Unsigned auto h>
constexpr decltype(auto) offset(Unsigned auto inst) {
  return (inst >> l) & fillbits(diff<l, h>::value);
}

constexpr static uint32_t sign_bit_mask = 0x80000000;

template <typename T>
concept Enum = std::is_enum<T>::value;

template <Enum T> constexpr std::underlying_type_t<T> to_int(T t) {
  return static_cast<std::underlying_type_t<T>>(t);
}

constexpr auto sign_extend(Integral auto x, Unsigned auto shamt) {
  return static_cast<decltype(x)>(
      static_cast<std::make_signed_t<decltype(x)>>(x) >> shamt);
}

constexpr uint8_t unpack_rd(uint32_t word) { return offset<7u, 11u>(word); }
constexpr uint8_t unpack_rs1(uint32_t word) { return offset<15u, 19u>(word); }
constexpr uint8_t unpack_rs2(uint32_t word) { return offset<20u, 24u>(word); }

constexpr uint32_t pack_alu(uint8_t funct3, uint8_t funct7, uint8_t rd,
                            uint8_t rs1, uint8_t rs2) {
  return rd << 7 | funct3 << 12 | rs1 << 15 | rs2 << 20 | funct7 << 25 |
         to_int(opcode::reg_reg);
}

constexpr uint32_t unpack_imm_i(uint32_t word) {
  return offset<20u, 20u>(word) | (offset<21u, 30u>(word) << 1) |
         sign_extend(word & sign_bit_mask, 20u);
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
         sign_extend(word & sign_bit_mask, 11u);
}

constexpr uint32_t pack_branch_op(uint8_t funct3, uint8_t rs1, uint8_t rs2,
                                  uint32_t imm) {
  return offset<11u, 11u>(imm) << 7 | offset<1u, 4u>(imm) << 8 | funct3 << 12 |
         rs1 << 15 | rs2 << 20 | offset<5u, 11u>(imm) << 25 |
         offset<12u, 12u>(imm) << 31 | to_int(opcode::branch);
}

constexpr uint32_t unpack_imm_b(uint32_t word) {
  return (offset<8u, 11u>(word) << 1) | (offset<25u, 30u>(word) << 5) |
         (offset<7u, 7u>(word) << 11) | sign_extend(word & sign_bit_mask, 19u);
}

constexpr uint32_t pack_store_op(uint8_t funct3, uint8_t rs1, uint8_t rs2,
                                 uint32_t imm) {
  return offset<0u, 4u>(imm) << 7 | funct3 << 12 | rs1 << 15 | rs2 << 20 |
         offset<5u, 11u>(imm) << 25 | to_int(opcode::store);
};

constexpr uint32_t unpack_imm_s(uint32_t word) {
  return offset<7u, 11u>(word) | (offset<25u, 30u>(word) << 5) |
         sign_extend(word & sign_bit_mask, 20u);
}
constexpr uint32_t unpack_csr(uint32_t word) { return offset<20u, 31u>(word); }

struct rv32_isn {
  virtual void     unpack(uint32_t word) = 0;
  virtual uint32_t pack() const          = 0;
  virtual ~rv32_isn()                    = default;
};

#define RV32_REG_REG_INST(name, funct3, funct7)                                    \
  struct rv32_##name : public rv32_isn {                                       \
    uint8_t rd;                                                                \
    uint8_t rs1;                                                               \
    uint8_t rs2;                                                               \
    explicit(false) rv32_##name(uint32_t word) { unpack(word); }               \
    void unpack(uint32_t word) final {                                         \
      rd  = unpack_rd(word);                                                   \
      rs1 = unpack_rs1(word);                                                  \
      rs2 = unpack_rs2(word);                                                  \
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
      rd  = unpack_rd(word);                                                   \
      rs  = unpack_rs1(word);                                                  \
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

struct rv32_slli : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  explicit(false) rv32_slli(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) final {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_rs2(word);
  }
  uint32_t pack() const final {
    return pack_i_op(static_cast<uint8_t>(reg_imm::slli), rd, rs, imm,
                     opcode::reg_imm);
  }
  explicit(false) operator uint32_t() const { return pack(); }
};

struct rv32_srli : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  explicit(false) rv32_srli(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) final {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_rs2(word);
  }
  uint32_t pack() const final {
    return pack_i_op(static_cast<uint8_t>(reg_imm::srli_srai), rd, rs, imm,
                     opcode::reg_imm);
  }
  explicit(false) operator uint32_t() const { return pack(); }
};

struct rv32_srai : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  explicit(false) rv32_srai(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) final {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_rs2(word);
  }
  uint32_t pack() const final {
    return pack_i_op(static_cast<uint8_t>(reg_imm::srli_srai), rd, rs, imm,
                     opcode::reg_imm) |
           (0b0100000 << 25);
  }
  explicit(false) operator uint32_t() const { return pack(); }
};

struct rv32_lui : public rv32_isn {
  uint8_t  rd;
  uint32_t imm;
  explicit(false) rv32_lui(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) final {
    rd  = unpack_rd(word);
    imm = unpack_imm_u(word);
  }
  uint32_t pack() const final {
    return pack_imm_u(rd, imm, static_cast<uint8_t>(opcode::lui));
  }
  explicit(false) operator uint32_t() const { return pack(); }
};

struct rv32_auipc : public rv32_isn {
  uint8_t  rd;
  uint32_t imm;
  explicit(false) rv32_auipc(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) final {
    rd  = unpack_rd(word);
    imm = unpack_imm_u(word);
  }
  uint32_t pack() const final {
    return pack_imm_u(rd, imm, static_cast<uint8_t>(opcode::auipc));
  }
  explicit(false) operator uint32_t() const { return pack(); }
};

struct rv32_jal : public rv32_isn {
  uint8_t  rd;
  uint32_t imm;
  explicit(false) rv32_jal(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) final {
    rd  = unpack_rd(word);
    imm = unpack_imm_j(word);
  }
  uint32_t pack() const final {
    return pack_jump_op(rd, imm, to_int(opcode::jal));
  }
  explicit(false) operator uint32_t() const { return pack(); }
};

#define RV32_STORE_INST(name, funct3)                                          \
  struct rv32_##name : public rv32_isn {                                       \
    uint8_t  rs1;                                                              \
    uint8_t  rs2;                                                              \
    uint32_t imm;                                                              \
    explicit(false) rv32_##name(uint32_t word) { unpack(word); }               \
    void unpack(uint32_t word) final {                                         \
      rs1 = unpack_rs1(word);                                                  \
      rs2 = unpack_rs2(word);                                                  \
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
      rs1 = unpack_rs1(word);                                                  \
      rs2 = unpack_rs2(word);                                                  \
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
      rd  = unpack_rd(word);                                                   \
      rs  = unpack_rs1(word);                                                  \
      csr = unpack_csr(word);                                                  \
    }                                                                          \
    uint32_t pack() const final {                                              \
      return rd << 7 | to_int(funct3) << 12 | rs << 15 | csr << 20 |           \
             to_int(opcode::sys);                                          \
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
    uint8_t rd;                                                                \
    uint8_t rs1;                                                               \
                                                                               \
    explicit(false) rv32_##name(uint32_t word) { unpack(word); }               \
    void unpack(uint32_t word) final {                                         \
      rd  = unpack_rd(word);                                                   \
      rs1 = unpack_rs1(word);                                                  \
    }                                                                          \
    uint32_t pack() const final {                                              \
      return rd << 7 | 0x0 << 12 | rs1 << 15 | to_int(rs2) << 20 |             \
             funct7 << 25 | to_int(opcode::sys);                           \
    }                                                                          \
    explicit(false) operator uint32_t() const { return pack(); }               \
  };

RV32_TRAP_ENV_INST(ecall, other_sys::ecall, 0x0)
RV32_TRAP_ENV_INST(ebreak, other_sys::ebreak, 0x0)
RV32_TRAP_ENV_INST(uret, other_sys::trap_ret, 0x0)
RV32_TRAP_ENV_INST(sret, other_sys::trap_ret, 0x8)
RV32_TRAP_ENV_INST(mret, other_sys::trap_ret, 0x24)
RV32_TRAP_ENV_INST(wfi, other_sys::interrupt_management, 0x8)

#undef RV32_TRAP_ENV_INST
