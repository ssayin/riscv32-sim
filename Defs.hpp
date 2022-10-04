#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <exception>
#include <utility>

#include <iostream>

#include "Util.hpp"

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
enum class ALU : std::uint32_t {
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

enum class Dummy : std::uint32_t {};

template <typename T>
concept Enum = std::is_enum<T>::value;

auto sign_extend(std::integral auto x, Unsigned auto shamt) {
  return static_cast<decltype(x)>(
      static_cast<std::make_signed_t<decltype(x)>>(x) >> shamt);
}

template <typename CRTP, Enum Funct3 = Dummy> class InstType {
  std::uint32_t inst;

  InstType(std::uint32_t x) : inst{x} {}
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

  template <std::size_t MemSize> friend class Computer;
};

class ALUInst : public InstType<ALUInst, ALU> {};
class BranchInst : public InstType<BranchInst, Branch> {};
class LoadInst : public InstType<LoadInst, Load> {};
class StoreInst : public InstType<StoreInst, Store> {};
class ImmediateInst : public InstType<ImmediateInst, Immediate> {};
class CSREnvInst : public InstType<CSREnvInst, Csr_Env> {};
class FenceInst : public InstType<FenceInst, Fence> {};
class UImmediateInst : public InstType<UImmediateInst> {};
class UJumpInst : public InstType<UJumpInst> {};

class RegisterFile {
public:
  constexpr std::uint32_t &operator[](std::size_t index) {
    x[0] = 0u;
    if (index >= 32u)
      throw std::runtime_error("reg addr must be less than 32");
    return x[index];
  }

  constexpr const std::uint32_t &operator[](std::size_t index) const {
    if (index >= 32u)
      throw std::runtime_error("reg addr must be less than 32");

    return x[index];
  }

private:
  std::array<std::uint32_t, 32> x{};
};

template <std::size_t MemSize = 128> struct Computer {
  static_assert(MemSize >= 0 &&
                MemSize <= std::numeric_limits<std::uint32_t>::max());
  std::uint32_t                     PC;
  RegisterFile                      x{};
  std::array<std::uint8_t, MemSize> Mem{};

  std::uint8_t  read_byte(std::size_t off) { return Mem.at(off); }
  std::uint16_t read_half(std::size_t off) {
    return read_byte(off) | (read_byte(off + 1) << 8);
  }
  std::uint32_t read_word(std::size_t off) {
    return read_half(off) | (read_half(off + 2) << 16);
  }

  std::uint8_t  write_byte() {}
  std::uint16_t write_half() {}
  std::uint32_t write_word() {}

  void exec(std::uint32_t inst) {
    switch (static_cast<OpCode>(offset<0u, 6u>(inst))) {

    case OpCode::ALU:
      exec(ALUInst{inst});
      break;

    case OpCode::Load:
      exec(LoadInst{inst});
      break;

    case OpCode::Store:
      exec(StoreInst{inst});
      break;

    case OpCode::Csr_Env:
      exec(CSREnvInst{inst});
      break;

    case OpCode::AUIPC:
    case OpCode::LUI:
      exec(UImmediateInst{inst});
      break;

    case OpCode::Immediate:
      exec(ImmediateInst{inst});
      break;

    case OpCode::Fence:
      exec(FenceInst{inst});
      break;

    case OpCode::JAL:
    case OpCode::JALR:
      exec(UJumpInst{inst});
      break;

    case OpCode::Branch:
      exec(BranchInst{inst});
      break;

    default:
      break;
    }
  }

  void exec(ALUInst inst) {
    switch (inst.funct3()) {
      using enum ALU;
    case AND:
      x[inst.rd()] = x[inst.rs1()] & x[inst.rs2()];
      break;
    case OR:
      x[inst.rd()] = x[inst.rs1()] | x[inst.rs2()];
      break;
    case XOR:
      x[inst.rd()] = x[inst.rs1()] ^ x[inst.rs2()];
      break;

    case ADD_SUB:
      switch (inst.bit30()) {
      case 0:
        x[inst.rd()] = x[inst.rs1()] + x[inst.rs2()];
        break;
      case 1:
        x[inst.rd()] = x[inst.rs1()] - x[inst.rs2()];
        break;
      }
      break;

    case SLL:
      x[inst.rd()] = x[inst.rs1()] << x[inst.rs2()];
      break;

    case SRL_SRA:
      switch (inst.bit30()) {
      case 0:
        x[inst.rd()] = x[inst.rs1()] >> x[inst.rs2()];
        break;
      case 1:
        x[inst.rd()] = sign_extend(x[inst.rs1()], x[inst.rs2()]);
        break;
      }
      break;

    case SLT:
      x[inst.rd()] = static_cast<std::int32_t>(x[inst.rs1()]) <
                     static_cast<std::int32_t>(x[inst.rs2()]);
      break;

    case SLTU:
      x[inst.rd()] = x[inst.rs1()] < x[inst.rs2()];
      break;
    }
  }

  void exec(ImmediateInst inst) {
    using enum Immediate;
    switch (inst.funct3()) {
    case ADDI:
      x[inst.rd()] = x[inst.rs1()] + inst.imm_i();
      break;
    case SLTI:
      x[inst.rd()] = static_cast<std::int32_t>(x[inst.rs1()]) <
                     static_cast<std::int32_t>(inst.imm_i());
      break;
    case SLTIU:
      x[inst.rd()] = x[inst.rs1()] < inst.imm_i();
      break;
    case XORI:
      x[inst.rd()] = x[inst.rs1()] ^ inst.imm_i();
      break;
    case ORI:
      x[inst.rd()] = x[inst.rs1()] | inst.imm_i();
      break;
    case ANDI:
      x[inst.rd()] = x[inst.rs1()] & inst.imm_i();
      break;
    case SLLI:
      x[inst.rd()] = x[inst.rs1()] << offset<0u, 4u>(inst.imm_i());
      break;
    case SRLI_SRAI:
      switch (inst.bit30()) {
      case 0:
        x[inst.rd()] = x[inst.rs1()] >> offset<0u, 4u>(inst.imm_i());
        break;
      case 1:
        x[inst.rd()] = sign_extend(x[inst.rs1()], offset<0u, 4u>(inst.imm_i()));
        break;
      }
      break;
    }
  }
  void exec(UImmediateInst inst) {
    if (inst.opc() == OpCode::LUI) {
      x[inst.rd()] = inst.imm_u();
    } else if (inst.opc() == OpCode::AUIPC) {
      x[inst.rd()] = inst.imm_u() + PC;
    }
  }

  void exec(UJumpInst inst) {
    if (inst.opc() == OpCode::JAL) {
      x[inst.rd()] = PC + 4;
      PC           = PC + inst.imm_j();
    } else if (inst.opc() == OpCode::JALR) {
      x[inst.rd()] = PC + 4;
      PC           = (x[inst.rs1()] + inst.imm_i()) & 0xFFFFFFFE;
    }
  }

  void exec(BranchInst inst) {
    bool take_jump = false;
    switch (inst.funct3()) {
      using enum Branch;
    case BEQ:
      take_jump = (x[inst.rs1()] == x[inst.rs2()]);
      break;
    case BNE:
      take_jump = (x[inst.rs1()] != x[inst.rs2()]);
      break;
    case BLT:
      take_jump = (static_cast<std::int32_t>(x[inst.rs1()]) <
                   static_cast<std::int32_t>(x[inst.rs2()]));
      break;
    case BLTU:
      take_jump = (x[inst.rs1()] < x[inst.rs2()]);
      break;
    case BGE:
      take_jump = (static_cast<std::int32_t>(x[inst.rs1()]) >=
                   static_cast<std::int32_t>(x[inst.rs2()]));
      break;
    case BGEU:
      take_jump = (x[inst.rs1()] >= x[inst.rs2()]);
      break;
    }

    if (take_jump)
      PC = PC + inst.imm_b();
  }

  void exec(LoadInst inst) {}
  void exec(StoreInst inst) {}

  void exec(FenceInst inst) {}
  void exec(CSREnvInst inst) {}
};
