#include "Defs.hpp"

std::uint8_t Computer::read_byte(std::size_t off) {
  if (off < MemSize)
    return Mem[off];
  else
    throw std::runtime_error("read_byte failed");
}
std::uint16_t Computer::read_half(std::size_t off) {
  return read_byte(off) | (read_byte(off + 1) << 8);
}
std::uint32_t Computer::read_word(std::size_t off) {
  return read_half(off) | (read_half(off + 2) << 16);
}

void Computer::write_byte(std::size_t off, std::uint8_t b) {
  if (off < MemSize) {
    Mem[off] = b;
  } else
    throw std::runtime_error("write_byte offset boundary check failed");
}

void Computer::write_half(std::size_t off, std::uint16_t h) {
  write_byte(off, offset<0u, 7u>(h));
  write_byte(off + 1, offset<8u, 15u>(h));
}

void Computer::write_word(std::size_t off, std::uint32_t w) {
  write_half(off, offset<0u, 15u>(w));
  write_half(off + 2, offset<16u, 31u>(w));
}

void Computer::exec(std::uint32_t inst) {
  PC_Next = PC + 4;
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
    throw std::runtime_error("unknown opcode");
    break;
  }

  PC = PC_Next;
}

void Computer::exec(ALUInst inst) {
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

void Computer::exec(ImmediateInst inst) {
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
void Computer::exec(UImmediateInst inst) {
  if (inst.opc() == OpCode::LUI) {
    x[inst.rd()] = inst.imm_u();
  } else if (inst.opc() == OpCode::AUIPC) {
    x[inst.rd()] = inst.imm_u() + PC;
  }
}

void Computer::exec(UJumpInst inst) {
  if (inst.opc() == OpCode::JAL) {
    x[inst.rd()] = PC + 4;
    PC_Next      = PC + inst.imm_j();
  } else if (inst.opc() == OpCode::JALR) {
    x[inst.rd()] = PC + 4;
    PC_Next      = (x[inst.rs1()] + inst.imm_i()) & 0xFFFFFFFE;
  }
}

void Computer::exec(BranchInst inst) {
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
    PC_Next = PC + inst.imm_b();
}

void Computer::exec(LoadInst inst) {
  using enum Load;
  std::uint32_t addr = x[inst.rs1()] + inst.imm_i();
  switch (inst.funct3()) {
  case LB:
    x[inst.rd()] = static_cast<std::uint32_t>(
        static_cast<std::int32_t>(read_byte(addr) << 24) >> 24);
    break;
  case LH:
    x[inst.rd()] = static_cast<std::uint32_t>(
        static_cast<std::int32_t>(read_half(addr) << 16) >> 16);
    break;
  case LW:
    x[inst.rd()] = read_word(addr);
    break;
  case LBU:
    x[inst.rd()] = (read_byte(addr) << 24);
    break;
  case LHU:
    x[inst.rd()] = (read_half(addr) << 16);
    break;
  }
}

void Computer::exec(StoreInst inst) {
  std::uint32_t addr = x[inst.rs1()] + inst.imm_s();
  switch (inst.funct3()) {
    using enum Store;
  case SB:
    write_byte(addr, x[inst.rs2()]);
    break;
  case SH:
    write_half(addr, x[inst.rs2()]);
    break;
  case SW:
    std::cout << std::hex << addr << " rs2 " << inst.rs1() << std::endl;
    write_word(addr, x[inst.rs2()]);
    break;
  }
}

void Computer::exec(FenceInst inst) {}
void Computer::exec(CSREnvInst inst) {}
