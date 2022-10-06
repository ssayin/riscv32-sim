#include "computer.hpp"
#include "Defs.hpp"

void Computer::exec(uint32_t inst) {
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
  uint32_t rd    = inst.rd();
  uint32_t rs1_v = regfile.read(inst.rs1());
  uint32_t rs2_v = regfile.read(inst.rs2());
  switch (inst.funct3()) {
    using enum ALU;
  case AND:
    regfile.write(rd, rs1_v & rs2_v);
    break;
  case OR:
    regfile.write(rd, rs1_v | rs2_v);
    break;
  case XOR:
    regfile.write(rd, rs1_v ^ rs2_v);
    break;

  case ADD_SUB:
    switch (inst.bit30()) {
    case 0:
      regfile.write(rd, rs1_v + rs2_v);
      break;
    case 1:
      regfile.write(rd, rs1_v - rs2_v);
      break;
    }
    break;

  case SLL:
    regfile.write(rd, rs1_v << rs2_v);
    break;

  case SRL_SRA:
    switch (inst.bit30()) {
    case 0:
      regfile.write(rd, rs1_v >> rs2_v);
      break;
    case 1:
      regfile.write(rd, sign_extend(rs1_v, rs2_v));
      break;
    }
    break;

  case SLT:
    regfile.write(rd,
                  static_cast<int32_t>(rs1_v) < static_cast<int32_t>(rs2_v));
    break;

  case SLTU:
    regfile.write(rd, rs1_v < rs2_v);
    break;
  }
}

void Computer::exec(ImmediateInst inst) {
  uint32_t rd   = inst.rd();
  uint32_t rs_v = regfile.read(inst.rs1());
  uint32_t imm  = inst.imm_i();
  switch (inst.funct3()) {
    using enum Immediate;
  case ADDI:
    regfile.write(rd, rs_v + imm);
    break;
  case SLTI:
    regfile.write(rd, static_cast<int32_t>(rs_v) < static_cast<int32_t>(imm));
    break;
  case SLTIU:
    regfile.write(rd, rs_v < imm);
    break;
  case XORI:
    regfile.write(rd, rs_v ^ imm);
    break;
  case ORI:
    regfile.write(rd, rs_v | imm);
    break;
  case ANDI:
    regfile.write(rd, rs_v & imm);
    break;
  case SLLI:
    regfile.write(rd, rs_v << offset<0u, 4u>(inst.imm_i()));
    break;
  case SRLI_SRAI:
    switch (inst.bit30()) {
    case 0:
      regfile.write(rd, rs_v >> offset<0u, 4u>(inst.imm_i()));
      break;
    case 1:
      regfile.write(rd, sign_extend(rs_v, offset<0u, 4u>(inst.imm_i())));
      break;
    }
    break;
  }
}
void Computer::exec(UImmediateInst inst) {
  if (inst.opc() == OpCode::LUI) {
    regfile.write(inst.rd(), inst.imm_u());
  } else if (inst.opc() == OpCode::AUIPC) {
    regfile.write(inst.rd(), inst.imm_u() + PC);
  }
}

void Computer::exec(UJumpInst inst) {
  if (inst.opc() == OpCode::JAL) {
    PC_Next = PC + inst.imm_j();
  } else if (inst.opc() == OpCode::JALR) {
    PC_Next = (regfile.read(inst.rs1()) + inst.imm_i()) & 0xFFFFFFFE;
  } else
    return;

  regfile.write(inst.rd(), PC + 4);
}

void Computer::exec(BranchInst inst) {
  bool     take_jump = false;
  uint32_t rs2_v     = regfile.read(inst.rs2());
  uint32_t rs1_v     = regfile.read(inst.rs1());
  switch (inst.funct3()) {
    using enum Branch;
  case BEQ:
    take_jump = (rs1_v == rs2_v);
    break;
  case BNE:
    take_jump = (rs1_v != rs2_v);
    break;
  case BLT:
    take_jump = (static_cast<int32_t>(rs1_v) < static_cast<int32_t>(rs2_v));
    break;
  case BLTU:
    take_jump = (rs1_v < rs2_v);
    break;
  case BGE:
    take_jump = (static_cast<int32_t>(rs1_v) >= static_cast<int32_t>(rs2_v));
    break;
  case BGEU:
    take_jump = (rs1_v >= rs2_v);
    break;
  }
  if (take_jump)
    PC_Next = PC + inst.imm_b();
}

void Computer::exec(LoadInst inst) {
  using enum Load;
  uint32_t rd   = inst.rd();
  uint32_t addr = regfile.read(inst.rs1()) + inst.imm_i();
  switch (inst.funct3()) {
  case LB:
    regfile.write(rd,
                  static_cast<uint32_t>(
                      static_cast<int32_t>(mem.read_byte(addr) << 24) >> 24));
    break;
  case LH:
    regfile.write(rd,
                  static_cast<uint32_t>(
                      static_cast<int32_t>(mem.read_half(addr) << 16) >> 16));
    break;
  case LW:
    regfile.write(rd, mem.read_word(addr));
    break;
  case LBU:
    regfile.write(rd, mem.read_byte(addr) << 24);
    break;
  case LHU:
    regfile.write(rd, mem.read_half(addr) << 16);
    break;
  }
}

void Computer::exec(StoreInst inst) {
  uint32_t addr = regfile.read(inst.rs1()) + inst.imm_s();
  uint32_t rs_v = regfile.read(inst.rs2());
  switch (inst.funct3()) {
    using enum Store;
  case SB:
    mem.write_byte(addr, rs_v);
    break;
  case SH:
    mem.write_half(addr, rs_v);
    break;
  case SW:
    mem.write_word(addr, rs_v);
    break;
  }
}

void Computer::exec(FenceInst inst) {}
void Computer::exec(CSREnvInst inst) {}
