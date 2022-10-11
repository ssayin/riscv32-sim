#include "rv32_decode.hpp"
#include "rv32_isn.hpp"
#include <iostream>

decoder_out decode_load(uint32_t word);
decoder_out decode_store(uint32_t word);
decoder_out decode_branch(uint32_t word);
decoder_out decode_immediate(uint32_t word);
decoder_out decode_alu(uint32_t word);
decoder_out decode_csrenv(uint32_t word);
decoder_out decode_fence(uint32_t word);

decoder_out decode(uint32_t word) {
  switch (static_cast<OpCode>(offset<0u, 6u>(word))) {
    using enum OpCode;
  case AUIPC: {
    rv32_auipc isn{word};
    return decoder_out(true, isn.rd, 0, 0, alu_type::AUIPC, pipeline_type::ALU,
                       isn.imm);
  }
  case LUI: {
    rv32_lui isn{word};
    return decoder_out(true, isn.rd, 0, 0, alu_type::ADD, pipeline_type::ALU,
                       isn.imm);
  }
  case JAL: {
    rv32_jal isn{word};
    return decoder_out(true, isn.rd, 0, 0, alu_type::JAL, pipeline_type::ALU,
                       isn.imm);
  }
  case JALR: {
    rv32_jalr isn{word};
    return decoder_out(true, isn.rd, isn.rs, 0, alu_type::JALR,
                       pipeline_type::ALU, isn.imm);
  }
  case Load: return decode_load(word);
  case Store: return decode_store(word);
  case Branch: return decode_branch(word);
  case Immediate: return decode_immediate(word);
  case ALU: return decode_alu(word);
  case Csr_Env: return decode_csrenv(word);
  case Fence: return decode_fence(word);
  default: throw std::runtime_error("unknown instruction type");
  }
}

decoder_out decode_load(uint32_t word) {
  switch (static_cast<Load>(offset<12u, 14u>(word))) {
    using enum Load;
  case LB: {
    rv32_lb isn{word};
    return decoder_out(true, isn.rd, isn.rs, 0, ls_type::LB, pipeline_type::LS,
                       isn.imm);
  }
  case LH: {
    rv32_lh isn{word};
    return decoder_out(true, isn.rd, isn.rs, 0, ls_type::LH, pipeline_type::LS,
                       isn.imm);
  }
  case LW: {
    rv32_lw isn{word};
    return decoder_out(true, isn.rd, isn.rs, 0, ls_type::LW, pipeline_type::LS,
                       isn.imm);
  }
  case LBU: {
    rv32_lbu isn{word};
    return decoder_out(true, isn.rd, isn.rs, 0, ls_type::LBU, pipeline_type::LS,
                       isn.imm);
  }
  case LHU: {
    rv32_lhu isn{word};
    return decoder_out(true, isn.rd, isn.rs, 0, ls_type::LHU, pipeline_type::LS,
                       isn.imm);
  }
  }
}

decoder_out decode_store(uint32_t word) {
  switch (static_cast<Store>(offset<12u, 14u>(word))) {
    using enum Store;
  case SB: {
    rv32_sb isn{word};
    return decoder_out(true, 0, isn.rs1, isn.rs2, ls_type::SB,
                       pipeline_type::LS, isn.imm);
  }
  case SH: {
    rv32_sh isn{word};
    return decoder_out(true, 0, isn.rs1, isn.rs2, ls_type::SH,
                       pipeline_type::LS, isn.imm);
  }
  case SW: {
    rv32_sw isn{word};
    return decoder_out(true, 0, isn.rs1, isn.rs2, ls_type::SW,
                       pipeline_type::LS, isn.imm);
  }
  }
}

decoder_out decode_alu(uint32_t word) {
  switch (static_cast<ALU>(offset<12u, 14u>(word))) {
    using enum ALU;
  case AND: {
    rv32_and isn{word};
    return decoder_out(false, isn.rd, isn.rs1, isn.rs2, alu_type::AND,
                       pipeline_type::ALU, 0);
  }
  case OR: {
    rv32_or isn{word};
    return decoder_out(false, isn.rd, isn.rs1, isn.rs2, alu_type::OR,
                       pipeline_type::ALU, 0);
  }
  case XOR: {
    rv32_xor isn{word};
    return decoder_out(false, isn.rd, isn.rs1, isn.rs2, alu_type::XOR,
                       pipeline_type::ALU, 0);
  }
  case ADD_SUB:
    switch (offset<30u, 30u>(word)) {
    case 0: {
      rv32_add isn{word};
      return decoder_out(false, isn.rd, isn.rs1, isn.rs2, alu_type::ADD,
                         pipeline_type::ALU, 0);
    }
    case 1: {
      rv32_sub isn{word};
      return decoder_out(false, isn.rd, isn.rs1, isn.rs2, alu_type::SUB,
                         pipeline_type::ALU, 0);
    }
    }
  case SLL: {
  }
  case SRL_SRA:
    switch (offset<30u, 30u>(word)) {
    case 0: {
      rv32_srl isn{word};
      return decoder_out(false, isn.rd, isn.rs1, isn.rs2, alu_type::SRL,
                         pipeline_type::ALU, 0);
    }
    case 1: {
      rv32_sra isn{word};
      return decoder_out(false, isn.rd, isn.rs1, isn.rs2, alu_type::SRA,
                         pipeline_type::ALU, 0);
    }
    }
  case SLT: {
    rv32_slt isn{word};
    return decoder_out(false, isn.rd, isn.rs1, isn.rs2, alu_type::SLT,
                       pipeline_type::ALU, 0);
  }
  case SLTU: {
    rv32_sltu isn{word};
    return decoder_out(false, isn.rd, isn.rs1, isn.rs2, alu_type::SLTU,
                       pipeline_type::ALU, 0);
  }
  }
}

decoder_out decode_immediate(uint32_t word) {
  switch (static_cast<Immediate>(offset<12u, 14u>(word))) {
    using enum Immediate;
  case ADDI: {
    rv32_addi isn{word};
    return decoder_out(true, isn.rd, isn.rs, 0, alu_type::ADD,
                       pipeline_type::ALU, isn.imm);
  }
  case SLTI: {
    rv32_slti isn{word};
    return decoder_out(true, isn.rd, isn.rs, 0, alu_type::SLT,
                       pipeline_type::ALU, isn.imm);
  }

  case SLTIU: {
    rv32_sltiu isn{word};
    return decoder_out(true, isn.rd, isn.rs, 0, alu_type::SLTU,
                       pipeline_type::ALU, isn.imm);
  }

  case XORI: {
    rv32_xori isn{word};
    return decoder_out(true, isn.rd, isn.rs, 0, alu_type::XOR,
                       pipeline_type::ALU, isn.imm);
  }

  case ORI: {
    rv32_ori isn{word};
    return decoder_out(true, isn.rd, isn.rs, 0, alu_type::OR,
                       pipeline_type::ALU, isn.imm);
  }

  case ANDI: {
    rv32_andi isn{word};
    return decoder_out(true, isn.rd, isn.rs, 0, alu_type::AND,
                       pipeline_type::ALU, isn.imm);
  }

  case SRLI_SRAI:
    switch (offset<30u, 30u>(word)) {
    case 0: {
      rv32_srli isn{word};
      return decoder_out(true, isn.rd, isn.rs, 0, alu_type::SRL,
                         pipeline_type::ALU, isn.imm);
    }
    case 1: {
      rv32_srai isn{word};
      return decoder_out(true, isn.rd, isn.rs, 0, alu_type::SRA,
                         pipeline_type::ALU, isn.imm);
    }
    }

  case SLLI: {
    rv32_slli isn{word};
    return decoder_out(true, isn.rd, isn.rs, 0, alu_type::SLL,
                       pipeline_type::ALU, isn.imm);
  }
  }
}

decoder_out decode_branch(uint32_t word) {
  switch (static_cast<Branch>(offset<12u, 14u>(word))) {
    using enum Branch;
  case BEQ: {
    rv32_beq isn{word};
    return decoder_out(true, 0, isn.rs1, isn.rs2, branch_type::BEQ,
                       pipeline_type::BRANCH, isn.imm);
  }
  case BNE: {
    rv32_bne isn{word};
    return decoder_out(true, 0, isn.rs1, isn.rs2, branch_type::BNE,
                       pipeline_type::BRANCH, isn.imm);
  }
  case BLT: {
    rv32_blt isn{word};
    return decoder_out(true, 0, isn.rs1, isn.rs2, branch_type::BLT,
                       pipeline_type::BRANCH, isn.imm);
  }
  case BLTU: {
    rv32_bltu isn{word};
    return decoder_out(true, 0, isn.rs1, isn.rs2, branch_type::BLTU,
                       pipeline_type::BRANCH, isn.imm);
  }
  case BGE: {
    rv32_bge isn{word};
    return decoder_out(true, 0, isn.rs1, isn.rs2, branch_type::BGE,
                       pipeline_type::BRANCH, isn.imm);
  }
  case BGEU: {
    rv32_bgeu isn{word};
    return decoder_out(true, 0, isn.rs1, isn.rs2, branch_type::BGEU,
                       pipeline_type::BRANCH, isn.imm);
  }
  }
}

decoder_out decode_fence(uint32_t word) {
  std::cout << "fence ops are not implemented" << std::endl;
  // insert NOP
  return decoder_out(true, 0, 0, 0, alu_type::ADD, pipeline_type::ALU, 0);
}

decoder_out decode_csrenv(uint32_t word) {
  std::cout << "csr and env ops are not implemented" << std::endl;
  // insert NOP
  return decoder_out(true, 0, 0, 0, alu_type::ADD, pipeline_type::ALU, 0);
}
