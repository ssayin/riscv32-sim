#pragma once

#include <bitset>
#include <cstdint>
#include <numeric>
#include <type_traits>
#include <utility>

enum class OpCode : uint32_t {
  Branch    = 0b1100011,
  Load      = 0b0000011,
  Store     = 0b0100011,
  Immediate = 0b0010011,
  ALU       = 0b0110011,
  Fence     = 0b0001111,
  Csr_Env   = 0b1110011,
  LUI       = 0b0110111, // U-type
  AUIPC     = 0b0010111, // U-type
  JAL       = 0b1101111, // J-type
  JALR      = 0b1100111, // I-type
};

// B-Type
enum class Branch : uint32_t {
  BEQ  = 0b000,
  BNE  = 0b001,
  BLT  = 0b100,
  BGE  = 0b101,
  BLTU = 0b110,
  BGEU = 0b111
};

// I-type
enum class Load : uint32_t {
  LB  = 0b000,
  LH  = 0b001,
  LW  = 0b010,
  LBU = 0b100,
  LHU = 0b101
};

// S-type
enum class Store : uint32_t {
  SB = 0b000,
  SH = 0b001,
  SW = 0b010,
};

// I-type
enum class Immediate : uint32_t {
  ADDI      = 0b000,
  SLTI      = 0b010,
  SLTIU     = 0b011,
  XORI      = 0b100,
  ORI       = 0b110,
  ANDI      = 0b111,
  SLLI      = 0b001,
  SRLI_SRAI = 0b101,
};

// R-type
enum class ALU : uint32_t {
  SLL     = 0b001,
  SRL_SRA = 0b101,
  ADD_SUB = 0b000,
  SLT     = 0b010,
  SLTU    = 0b011,
  XOR     = 0b100,
  OR      = 0b110,
  AND     = 0b111,
};

// I-type
enum class Fence : uint32_t {
  FENCE  = 0b000,
  FENCEI = 0b001,
};

// I-type
enum class Csr_Env : uint32_t {
  ECALL_EBREAK = 0b000,
  CSRRW        = 0b001,
  CSRRS        = 0b010,
  CSRRC        = 0b011,
  CSRRWI       = 0b101,
  CSRRSI       = 0b110,
  CSRRCI       = 0b111,
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
         to_int(OpCode::ALU);
}

constexpr uint32_t unpack_imm_i(uint32_t word) {
  return offset<20u, 20u>(word) | (offset<21u, 30u>(word) << 1) |
         (word & sign_bit_mask) >> 20;
}

constexpr uint32_t pack_imm_op(uint8_t funct3, uint8_t rd, uint8_t rs,
                               uint32_t imm) {
  return rd << 7 | funct3 << 12 | rs << 15 | imm << 20 |
         to_int(OpCode::Immediate);
}

constexpr uint32_t pack_load_op(uint8_t funct3, uint8_t rd, uint8_t rs,
                                uint32_t imm) {
  return rd << 7 | funct3 << 12 | rs << 15 | imm << 20 | to_int(OpCode::Load);
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
         offset<12u, 12u>(imm) << 31 | to_int(OpCode::Branch);
}

constexpr uint32_t unpack_imm_b(uint32_t word) {
  return (offset<8u, 11u>(word) << 1) | (offset<25u, 30u>(word) << 5) |
         (offset<7u, 7u>(word) << 11) | sign_extend(word & sign_bit_mask, 19u);
}

constexpr uint32_t pack_store_op(uint8_t funct3, uint8_t rs1, uint8_t rs2,
                                 uint32_t imm) {
  return offset<0u, 4u>(imm) << 7 | funct3 << 12 | rs1 << 15 | rs2 << 20 |
         offset<5u, 11u>(imm) << 25 | to_int(OpCode::Store);
};

constexpr uint32_t unpack_imm_s(uint32_t word) {
  return offset<7u, 11u>(word) | (offset<25u, 30u>(word) << 5) |
         sign_extend(word & sign_bit_mask, 20u);
}
