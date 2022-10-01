#pragma once

#include <cstdint>
#include <variant>

enum class OpCode : std::uint32_t {
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
enum class Branch : std::uint32_t {
  BEQ  = 0b000,
  BNE  = 0b001,
  BLT  = 0b100,
  BGE  = 0b101,
  BLTU = 0b110,
  BGEU = 0b111
};

// I-type
enum class Load : std::uint32_t {
  LB  = 0b000,
  LH  = 0b001,
  LW  = 0b010,
  LBU = 0b100,
  LHU = 0b101
};

// S-type
enum class Store : std::uint32_t {
  SB = 0b000,
  SH = 0b001,
  SW = 0b010,
};

// I-type
enum class Immediate : std::uint32_t {
  ADDI  = 0b000,
  SLTI  = 0b010,
  SLTIU = 0b011,
  XORI  = 0b100,
  ORI   = 0b110,
  ANDI  = 0b111,
};

// R-type
enum class ALU : std::uint32_t {
  SLL_SLLI          = 0b001,
  SRL_SRLI_SRA_SRAI = 0b101,
  ADD_SUB           = 0b000,
  SLT               = 0b010,
  SLTU              = 0b011,
  XOR               = 0b100,
  OR                = 0b110,
  AND               = 0b111,
};

// I-type
enum class Fence : std::uint32_t {
  FENCE  = 0b000,
  FENCEI = 0b001,
};

// I-type
enum class Csr_Env : std::uint32_t {
  ECALL_EBREAK = 0b000,
  CSRRW        = 0b001,
  CSRRS        = 0b010,
  CSRRC        = 0b011,
  CSRRWI       = 0b101,
  CSRRSI       = 0b110,
  CSRRCI       = 0b111,
};

template <typename CRTP> struct InstructionType { std::uint32_t inst; };

struct RType : InstructionType<RType> {};
struct IType : InstructionType<IType> {};
struct SType : InstructionType<SType> {};
struct BType : InstructionType<BType> {};
struct UType : InstructionType<UType> {};
struct JType : InstructionType<JType> {};

using Instruction =
    std::variant<std::monostate, RType, IType, SType, BType, UType, JType>;
