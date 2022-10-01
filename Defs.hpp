#pragma once

#include <cstdint>

enum class OpCode : std::uint32_t {
  Branch    = 0b1100011,
  Load      = 0b0000011,
  Store     = 0b0100011,
  Immediate = 0b0010011,
  ALU       = 0b0110011,
  Fence     = 0b0001111,
  CsrEnv    = 0b1110011,
  LUI       = 0b0110111,
  AUIPC     = 0b0010111,
  JAL       = 0b1101111,
  JALR      = 0b1100111,
};

template <typename CRTP> struct Instruction { std::uint32_t inst; };

struct RType : Instruction<RType> {};
struct IType : Instruction<IType> {};
struct SType : Instruction<SType> {};
struct BType : Instruction<BType> {};
struct UType : Instruction<UType> {};
struct JType : Instruction<JType> {};
