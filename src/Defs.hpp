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

struct rv32_addi {};
struct rv32_slti {};
struct rv32_sltiu {};
struct rv32_xori {};
struct rv32_ori {};
struct rv32_andi {};
struct rv32_slli {};
struct rv32_srli {};
struct rv32_srai {};

struct rv32_lui {};
struct rv32_auipc {};

struct rv32_jal {};
struct rv32_jalr {};

struct rv32_sb {};
struct rv32_sh {};
struct rv32_sw {};

struct rv32_beq {};
struct rv32_bne {};
struct rv32_blt {};
struct rv32_bge {};
struct rv32_bltu {};
struct rv32_bgeu {};

struct rv32_lb {};
struct rv32_lh {};
struct rv32_lw {};
struct rv32_lbu {};
struct rv32_lhu {};
/*
 * ### OPTION 1 ###
 */
constexpr uint8_t unpack_rd(uint32_t word) { return offset<7u, 11u>(word); }
constexpr uint8_t unpack_rs1(uint32_t word) { return offset<15u, 19u>(word); }
constexpr uint8_t unpack_rs2(uint32_t word) { return offset<20u, 24u>(word); }

constexpr uint32_t pack_alu(uint8_t funct3, uint8_t funct7, uint8_t rd,
                            uint8_t rs1, uint8_t rs2) {
  return static_cast<uint32_t>(rd) << 7 | static_cast<uint32_t>(funct3) << 12 |
         static_cast<uint32_t>(rs1) << 15 | static_cast<uint32_t>(rs2) << 20 |
         static_cast<uint32_t>(funct7) << 25 | 0b0110011;
}

struct rv32_sll {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  constexpr rv32_sll(uint32_t word)
      : rd(unpack_rd(word)), rs1(unpack_rs1(word)), rs2(unpack_rs2(word)) {}
  constexpr uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::SLL), 0x0, rd, rs1, rs2);
  }
  constexpr uint32_t operator()() const { return pack(); }
};

struct rv32_srl {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  constexpr rv32_srl(uint32_t word)
      : rd(unpack_rd(word)), rs1(unpack_rs1(word)), rs2(unpack_rs2(word)) {}
  constexpr uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::SRL_SRA), 0x0, rd, rs1, rs2);
  }
  constexpr uint32_t operator()() const { return pack(); }
};

struct rv32_sra {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  constexpr rv32_sra(uint32_t word)
      : rd(unpack_rd(word)), rs1(unpack_rs1(word)), rs2(unpack_rs2(word)) {}
  constexpr uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::SRL_SRA), 0x20, rd, rs1, rs2);
  }

  constexpr uint32_t operator()() const { return pack(); }
};

struct rv32_add {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  constexpr rv32_add(uint32_t word)
      : rd(unpack_rd(word)), rs1(unpack_rs1(word)), rs2(unpack_rs2(word)) {}
  constexpr uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::ADD_SUB), 0x0, rd, rs1, rs2);
  }
  constexpr uint32_t operator()() const { return pack(); }
};

struct rv32_sub {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  constexpr rv32_sub(uint32_t word)
      : rd(unpack_rd(word)), rs1(unpack_rs1(word)), rs2(unpack_rs2(word)) {}
  constexpr uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::ADD_SUB), 0x20, rd, rs1, rs2);
  }
  constexpr uint32_t operator()() const { return pack(); }
};

struct rv32_slt {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  constexpr rv32_slt(uint32_t word)
      : rd(unpack_rd(word)), rs1(unpack_rs1(word)), rs2(unpack_rs2(word)) {}
  constexpr uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::SLT), 0x0, rd, rs1, rs2);
  }
  constexpr uint32_t operator()() const { return pack(); }
};

struct rv32_sltu {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  constexpr rv32_sltu(uint32_t word)
      : rd(unpack_rd(word)), rs1(unpack_rs1(word)), rs2(unpack_rs2(word)) {}
  constexpr uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::SLTU), 0x0, rd, rs1, rs2);
  }

  constexpr uint32_t operator()() const { return pack(); }
};

struct rv32_xor {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  constexpr rv32_xor(uint32_t word)
      : rd(unpack_rd(word)), rs1(unpack_rs1(word)), rs2(unpack_rs2(word)) {}
  constexpr uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::XOR), 0x0, rd, rs1, rs2);
  }
  constexpr uint32_t operator()() const { return pack(); }
};

struct rv32_or {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  constexpr rv32_or(uint32_t word)
      : rd(unpack_rd(word)), rs1(unpack_rs1(word)), rs2(unpack_rs2(word)) {}
  constexpr uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::OR), 0x0, rd, rs1, rs2);
  }
  constexpr uint32_t operator()() const { return pack(); }
};

struct rv32_and {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  constexpr rv32_and(uint32_t word)
      : rd(unpack_rd(word)), rs1(unpack_rs1(word)), rs2(unpack_rs2(word)) {}
  constexpr uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::AND), 0x0, rd, rs1, rs2);
  }
  constexpr uint32_t operator()() const { return pack(); }
};

/*
 * ### OPTION 2 ###
 *
 * PLEASE CHOOSE ONE
 */

template <ALU funct3, uint8_t funct7 = 0x0> struct alu_op {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  constexpr alu_op(uint32_t word)
      : rd(unpack_rd(word)), rs1(unpack_rs1(word)), rs2(unpack_rs2(word)) {}

  constexpr uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(funct3), funct7, rd, rs1, rs2);
  }
  constexpr uint32_t operator()() const { return pack(); }
};

typedef alu_op<ALU::ADD_SUB>       rv32_alu_add;
typedef alu_op<ALU::ADD_SUB, 0x20> rv32_alu_sub;
typedef alu_op<ALU::XOR>           rv32_alu_xor;
typedef alu_op<ALU::SLL>           rv32_alu_sll;
typedef alu_op<ALU::SLT>           rv32_alu_slt;
typedef alu_op<ALU::SLTU>          rv32_alu_sltu;
typedef alu_op<ALU::OR>            rv32_alu_or;
typedef alu_op<ALU::AND>           rv32_alu_and;
typedef alu_op<ALU::SRL_SRA>       rv32_alu_srl;
typedef alu_op<ALU::SRL_SRA, 0x20> rv32_alu_sra;

/*
 * OPTION END
 */

enum class Dummy : uint32_t {};

template <typename T>
concept Enum = std::is_enum<T>::value;

template <typename T>
concept Integral = std::is_integral<T>::value;

auto sign_extend(Integral /* std::integral */ auto x, Unsigned auto shamt) {
  return static_cast<decltype(x)>(
      static_cast<std::make_signed_t<decltype(x)>>(x) >> shamt);
}

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
           sign_extend(inst & fillbits<0u>(32u), 11u);
  }

  auto imm_u() const {
    return (offset<12u, 30u>(inst) << 12) | (inst & fillbits<0u>(32u));
  }

  auto imm_b() const {
    return (offset<8u, 11u>(inst) << 1) | (offset<25u, 30u>(inst) << 5) |
           (offset<7u, 7u>(inst) << 11) |
           sign_extend(inst & fillbits<0u>(32u), 19u);
  }

  auto imm_s() const {
    return offset<7u, 11u>(inst) | (offset<25u, 30u>(inst) << 5) |
           sign_extend(inst & fillbits<0u>(32u), 20u);
  }

  auto imm_i() const {
    return offset<20u, 20u>(inst) | (offset<21u, 30u>(inst) << 1) |
           sign_extend(inst & fillbits<0u>(32u), 20u);
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
