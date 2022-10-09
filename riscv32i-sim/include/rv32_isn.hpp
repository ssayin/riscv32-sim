#pragma once

#include "Defs.hpp"
#include "computer.hpp"

struct rv32_isn {
  virtual void     unpack(uint32_t word) = 0;
  virtual uint32_t pack() const          = 0;
};

struct rv32_sll : public rv32_isn {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  rv32_sll(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
  }
  uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::SLL), 0x0, rd, rs1, rs2);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_srl : public rv32_isn {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  rv32_srl(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
  }

  uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::SRL_SRA), 0x0, rd, rs1, rs2);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_sra : public rv32_isn {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  rv32_sra(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
  }
  uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::SRL_SRA), 0x20, rd, rs1, rs2);
  }

  operator uint32_t() const { return pack(); }
};

struct rv32_add : public rv32_isn {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  rv32_add(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
  }
  uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::ADD_SUB), 0x0, rd, rs1, rs2);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_sub : public rv32_isn {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  rv32_sub(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
  }

  uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::ADD_SUB), 0x20, rd, rs1, rs2);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_slt : public rv32_isn {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  rv32_slt(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
  }

  uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::SLT), 0x0, rd, rs1, rs2);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_sltu : public rv32_isn {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  rv32_sltu(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
  }

  uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::SLTU), 0x0, rd, rs1, rs2);
  }

  operator uint32_t() const { return pack(); }
};

struct rv32_xor : public rv32_isn {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  rv32_xor(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
  }

  uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::XOR), 0x0, rd, rs1, rs2);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_or : public rv32_isn {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  rv32_or(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
  }

  uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::OR), 0x0, rd, rs1, rs2);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_and : public rv32_isn {
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;

  rv32_and(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
  }

  uint32_t pack() const {
    return pack_alu(static_cast<uint8_t>(ALU::AND), 0x0, rd, rs1, rs2);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_addi : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  rv32_addi(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_imm_i(word);
  }
  uint32_t pack() const {
    return pack_imm_op(static_cast<uint8_t>(Immediate::ADDI), rd, rs, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_xori : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  rv32_xori(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_imm_i(word);
  }

  uint32_t pack() const {
    return pack_imm_op(static_cast<uint8_t>(Immediate::XORI), rd, rs, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_ori : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  rv32_ori(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_imm_i(word);
  }

  uint32_t pack() const {
    return pack_imm_op(static_cast<uint8_t>(Immediate::ORI), rd, rs, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_andi : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  rv32_andi(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_imm_i(word);
  }

  uint32_t pack() const {
    return pack_imm_op(static_cast<uint8_t>(Immediate::ANDI), rd, rs, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_lb : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  rv32_lb(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_imm_i(word);
  }
  uint32_t pack() const {
    return pack_load_op(static_cast<uint8_t>(Load::LB), rd, rs, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_lh : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  rv32_lh(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_imm_i(word);
  }
  uint32_t pack() const {
    return pack_load_op(static_cast<uint8_t>(Load::LH), rd, rs, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_lw : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  rv32_lw(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_imm_i(word);
  }

  uint32_t pack() const {
    return pack_load_op(static_cast<uint8_t>(Load::LW), rd, rs, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_lbu : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  rv32_lbu(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_imm_i(word);
  }
  uint32_t pack() const {
    return pack_load_op(static_cast<uint8_t>(Load::LBU), rd, rs, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_lhu : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  rv32_lhu(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_imm_i(word);
  }
  uint32_t pack() const {
    return pack_load_op(static_cast<uint8_t>(Load::LHU), rd, rs, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_slli : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  rv32_slli(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_imm_i(word);
  }
  uint32_t pack() const {
    return pack_imm_op(static_cast<uint8_t>(Immediate::SLLI), rd, rs, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_srli : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  rv32_srli(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_imm_i(word);
  }
  uint32_t pack() const {
    return pack_imm_op(static_cast<uint8_t>(Immediate::SRLI_SRAI), rd, rs, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_srai : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  rv32_srai(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_imm_i(word);
  }
  uint32_t pack() const {
    return pack_imm_op(static_cast<uint8_t>(Immediate::SRLI_SRAI), rd, rs, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_slti : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  rv32_slti(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_imm_i(word);
  }
  uint32_t pack() const {
    return pack_imm_op(static_cast<uint8_t>(Immediate::SLTI), rd, rs, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_sltiu : public rv32_isn {
  uint8_t  rd;
  uint8_t  rs;
  uint32_t imm;
  rv32_sltiu(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    rs  = unpack_rs1(word);
    imm = unpack_imm_i(word);
  }
  uint32_t pack() const {
    return pack_imm_op(static_cast<uint8_t>(Immediate::SLTIU), rd, rs, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_lui : public rv32_isn {
  uint8_t  rd;
  uint32_t imm;
  rv32_lui(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    imm = unpack_imm_i(word);
  }
  uint32_t pack() const {
    return pack_imm_u(rd, imm, static_cast<uint8_t>(OpCode::LUI));
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_auipc : public rv32_isn {
  uint8_t  rd;
  uint32_t imm;
  rv32_auipc(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    imm = unpack_imm_i(word);
  }
  uint32_t pack() const {
    return pack_imm_u(rd, imm, static_cast<uint8_t>(OpCode::AUIPC));
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_jal : public rv32_isn {
  uint8_t  rd;
  uint32_t imm;
  rv32_jal(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rd  = unpack_rd(word);
    imm = unpack_imm_j(word);
  }
  uint32_t pack() const { return pack_jump_op(rd, imm, to_int(OpCode::JAL)); }
           operator uint32_t() const { return pack(); }
};

struct rv32_jalr : public rv32_isn {
  uint8_t  rd;
  uint32_t imm;
  rv32_jalr(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    imm = unpack_imm_j(word);
    rd  = unpack_rd(word);
  }
  uint32_t pack() const { return pack_jump_op(rd, imm, to_int(OpCode::JALR)); }
           operator uint32_t() const { return pack(); }
};

struct rv32_sb : public rv32_isn {
  uint8_t  rs1;
  uint8_t  rs2;
  uint32_t imm;
  rv32_sb(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
    imm = unpack_imm_s(word);
  }
  uint32_t pack() const {
    return pack_store_op(to_int(Store::SB), rs1, rs2, imm);
  }
  operator uint32_t() const { return pack(); }
};
struct rv32_sh : public rv32_isn {
  uint8_t  rs1;
  uint8_t  rs2;
  uint32_t imm;
  rv32_sh(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
    imm = unpack_imm_s(word);
  }
  uint32_t pack() const {
    return pack_store_op(to_int(Store::SH), rs1, rs2, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_sw : public rv32_isn {
  uint8_t  rs1;
  uint8_t  rs2;
  uint32_t imm;
  rv32_sw(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
    imm = unpack_imm_s(word);
  }
  uint32_t pack() const {
    return pack_store_op(to_int(Store::SW), rs1, rs2, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_beq : public rv32_isn {
  uint8_t  rs1;
  uint8_t  rs2;
  uint32_t imm;
  rv32_beq(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
    imm = unpack_imm_b(word);
  }
  uint32_t pack() const {
    return pack_branch_op(to_int(Branch::BEQ), rs1, rs2, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_bne : public rv32_isn {
  uint8_t  rs1;
  uint8_t  rs2;
  uint32_t imm;
  rv32_bne(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
    imm = unpack_imm_b(word);
  }
  uint32_t pack() const {
    return pack_branch_op(to_int(Branch::BNE), rs1, rs2, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_blt : public rv32_isn {
  uint8_t  rs1;
  uint8_t  rs2;
  uint32_t imm;
  rv32_blt(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
    imm = unpack_imm_b(word);
  }
  uint32_t pack() const {
    return pack_branch_op(to_int(Branch::BLT), rs1, rs2, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_bge : public rv32_isn {
  uint8_t  rs1;
  uint8_t  rs2;
  uint32_t imm;
  rv32_bge(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
    imm = unpack_imm_b(word);
  }
  uint32_t pack() const {
    return pack_branch_op(to_int(Branch::BGE), rs1, rs2, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_bltu : public rv32_isn {
  uint8_t  rs1;
  uint8_t  rs2;
  uint32_t imm;
  rv32_bltu(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
    imm = unpack_imm_b(word);
  }
  uint32_t pack() const {
    return pack_branch_op(to_int(Branch::BLTU), rs1, rs2, imm);
  }
  operator uint32_t() const { return pack(); }
};

struct rv32_bgeu : public rv32_isn {
  uint8_t  rs1;
  uint8_t  rs2;
  uint32_t imm;
  rv32_bgeu(uint32_t word) { unpack(word); }
  void unpack(uint32_t word) {
    rs1 = unpack_rs1(word);
    rs2 = unpack_rs2(word);
    imm = unpack_imm_b(word);
  }
  uint32_t pack() const {
    return pack_branch_op(to_int(Branch::BGEU), rs1, rs2, imm);
  }
  operator uint32_t() const { return pack(); }
};
