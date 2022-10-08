#pragma once

#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <exception>
#include <utility>

#include <iostream>
#include <memory>
#include <unordered_map>

#include "Util.hpp"

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

constexpr static uint32_t sign_bit_mask = 0x80000000;

constexpr auto sign_extend(Integral /* std::integral */ auto x,
                           Unsigned auto                     shamt) {
  return static_cast<decltype(x)>(
      static_cast<std::make_signed_t<decltype(x)>>(x) >> shamt);
}

constexpr uint8_t unpack_rd(uint32_t word) { return offset<7u, 11u>(word); }
constexpr uint8_t unpack_rs1(uint32_t word) { return offset<15u, 19u>(word); }
constexpr uint8_t unpack_rs2(uint32_t word) { return offset<20u, 24u>(word); }

constexpr uint32_t pack_alu(uint8_t funct3, uint8_t funct7, uint8_t rd,
                            uint8_t rs1, uint8_t rs2) {
  return static_cast<uint32_t>(rd) << 7 | static_cast<uint32_t>(funct3) << 12 |
         static_cast<uint32_t>(rs1) << 15 | static_cast<uint32_t>(rs2) << 20 |
         static_cast<uint32_t>(funct7) << 25 |
         static_cast<uint32_t>(OpCode::ALU);
}
constexpr uint32_t unpack_imm_i(uint32_t word) {
  return offset<20u, 20u>(word) | (offset<21u, 30u>(word) << 1) |
         static_cast<int32_t>(word & sign_bit_mask) >> 20;
}

constexpr uint32_t pack_imm_op(uint8_t funct3, uint8_t rd, uint8_t rs,
                               uint32_t imm) {
  return static_cast<uint32_t>(rd) << 7 | static_cast<uint32_t>(funct3) << 12 |
         static_cast<uint32_t>(rs) << 15 | (imm << 20) |
         static_cast<uint32_t>(OpCode::Immediate);
}

constexpr uint32_t pack_load_op(uint8_t funct3, uint8_t rd, uint8_t rs,
                                uint32_t imm) {
  return static_cast<uint32_t>(rd) << 7 | static_cast<uint32_t>(funct3) << 12 |
         static_cast<uint32_t>(rs) << 15 | (imm << 20) |
         static_cast<uint32_t>(OpCode::Load);
}

constexpr uint32_t unpack_imm_u(uint32_t word) {
  return (offset<12u, 30u>(word) << 12);
}

constexpr uint32_t pack_imm_u(uint8_t rd, uint32_t imm, uint8_t opc) {
  return (static_cast<uint32_t>(opc) | static_cast<uint32_t>(rd) << 7 | imm);
}

enum class Dummy : uint32_t {};

template <typename T>
concept Enum = std::is_enum<T>::value;

template <Enum Funct3 = Dummy> class InstType {
  uint32_t inst;

  InstType(uint32_t x) : inst{x} {}
  auto funct7() const { return offset<25u, 31u>(inst); }
  auto funct3() const { return static_cast<Funct3>(offset<12u, 14u>(inst)); }

  auto rs2() const { return offset<20u, 24u>(inst); }
  auto rs1() const { return offset<15u, 19u>(inst); }
  auto rd() const { return offset<7u, 11u>(inst); }

  auto bit30() const { return offset<30u, 30u>(inst); }

  OpCode opc() const { return static_cast<OpCode>(offset<0u, 6u>(inst)); }

  auto imm_j() const {
    return (offset<21u, 30u>(inst) << 1) | (offset<20u, 20u>(inst) << 11) |
           (offset<12u, 19u>(inst) << 12) |
           sign_extend(inst & sign_bit_mask, 11u);
  }

  auto imm_u() const {
    return (offset<12u, 30u>(inst) << 12) | (inst & sign_bit_mask);
  }

  auto imm_b() const {
    return (offset<8u, 11u>(inst) << 1) | (offset<25u, 30u>(inst) << 5) |
           (offset<7u, 7u>(inst) << 11) |
           sign_extend(inst & sign_bit_mask, 19u);
  }

  auto imm_s() const {
    return offset<7u, 11u>(inst) | (offset<25u, 30u>(inst) << 5) |
           sign_extend(inst & sign_bit_mask, 20u);
  }

  auto imm_i() const {
    return offset<20u, 20u>(inst) | (offset<21u, 30u>(inst) << 1) |
           sign_extend(inst & sign_bit_mask, 20u);
  }

  friend class Computer;
};

class ALUInst : public InstType<ALU> {};
class BranchInst : public InstType<Branch> {};
class LoadInst : public InstType<Load> {};
class StoreInst : public InstType<Store> {};
class ImmediateInst : public InstType<Immediate> {};
class CSREnvInst : public InstType<Csr_Env> {};
class FenceInst : public InstType<Fence> {};
class UImmediateInst : public InstType<> {};
class UJumpInst : public InstType<> {};