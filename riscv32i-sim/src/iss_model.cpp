#include "iss_model.hpp"
#include "rv32_decode.hpp"
#include "rv32_isn.hpp"

#include <iostream>

uint32_t reg_file::read(uint8_t index) {
  assert(index < 32u);
  return x[index];
}

void reg_file::write(uint8_t index, uint32_t data) {
  assert(index < 32u);
  if (index == 0)
    return;
  x[index] = data;
}

void iss_model::step() {
  uint32_t isn = mem.read_word(PC);
  std::cout << "PC " << std::hex << PC << " decode: " << std::hex << isn
            << std::endl;
  decoder_out dec = decode(isn);

  exec(dec);
  if (dec.target == pipeline_type::LS) {
    mem_phase(dec);
  }

  wb_retire_phase(dec);
}

void iss_model::exec(decoder_out &dec) {
  switch (dec.target) {
    using enum pipeline_type;
  case ALU: exec_alu(dec); break;
  case LS:
    alu_out = regfile.read(dec.rs1) + dec.imm; /* mem addr for load/store */
    break;
  case BRANCH: exec_alu_branch(dec); break;
  }
}

void iss_model::exec_alu(decoder_out &dec) {
  uint32_t opd1 = regfile.read(dec.rs1);
  uint32_t opd2 = dec.has_imm ? dec.imm : regfile.read(dec.rs2);

  switch (std::get<alu_type>(dec.op)) {
    using enum alu_type;
  case OR: alu_out = opd1 | opd2; break;
  case AND: alu_out = opd1 & opd2; break;
  case XOR: alu_out = opd1 ^ opd2; break;
  case ADD: alu_out = opd1 + opd2; break;
  case SUB: alu_out = opd1 - opd2; break;
  case SLL: alu_out = opd1 << opd2; break;
  case SRL: alu_out = opd1 >> opd2; break;
  case SRA: alu_out = sign_extend(opd1, opd2); break;
  case SLT:
    alu_out = static_cast<int32_t>(opd1) < static_cast<int32_t>(opd2);
    break;
  case SLTU: alu_out = opd1 < opd2; break;
  case AUIPC:
  case JAL: alu_out = PC + opd2; break;
  case JALR: alu_out = (opd1 + opd2) & 0xFFFFFFFE; break;
  }
}

void iss_model::exec_alu_branch(decoder_out &dec) {
  uint32_t opd1 = regfile.read(dec.rs1);
  uint32_t opd2 = regfile.read(dec.rs2);

  switch (std::get<branch_type>(dec.op)) {
    using enum branch_type;
  case BEQ: alu_out = opd1 == opd2; break;
  case BNE: alu_out = opd1 != opd2; break;
  case BLT:
    alu_out = static_cast<int32_t>(opd1) < static_cast<int32_t>(opd2);
    break;
  case BLTU: alu_out = opd1 < opd2; break;
  case BGE:
    alu_out = static_cast<int32_t>(opd1) >= static_cast<int32_t>(opd2);
    break;
  case BGEU: alu_out = opd1 >= opd2; break;
  }
}

uint32_t sign_extend(uint32_t in, uint8_t shamt) {
  return static_cast<uint32_t>(static_cast<int32_t>(in << shamt) >> shamt);
}

void iss_model::mem_phase(decoder_out &dec) {
  if (dec.target != pipeline_type::LS)
    return;
  terminate = (alu_out == tohost_addr);

  switch (std::get<ls_type>(dec.op)) {
    using enum ls_type;
  case LB: mem_out = sign_extend(mem.read_byte(alu_out), 24); break;
  case LH: mem_out = sign_extend(mem.read_half(alu_out), 16); break;
  case LW: mem_out = mem.read_word(alu_out); break;
  case LBU: mem_out = mem.read_byte(alu_out) << 24; break;
  case LHU: mem_out = mem.read_half(alu_out) << 16; break;
  case SB: mem.write_byte(alu_out, regfile.read(dec.rs2)); break;
  case SH: mem.write_half(alu_out, regfile.read(dec.rs2)); break;
  case SW: mem.write_word(alu_out, regfile.read(dec.rs2)); break;
  }
}

void iss_model::wb_retire_phase(decoder_out &dec) {
  switch (dec.target) {
    using enum pipeline_type;
  case LS:
    wb_retire_ls(dec);
    PC += 4;
    break;
    /* should skip mem_phase for branch and alu, so that alu_out will not be
     * overwritten */
  case ALU: wb_retire_alu(dec); break;
  case BRANCH:
    if (alu_out) {
      PC = PC + dec.imm;
    } else
      PC = PC + 4;
    break;
  }
}

void iss_model::wb_retire_ls(decoder_out &dec) {
  switch (std::get<ls_type>(dec.op)) {
    using enum ls_type;
  case LB:
  case LH:
  case LW:
  case LBU:
  case LHU: regfile.write(dec.rd, mem_out); break;
  default: break;
  }
}

void iss_model::wb_retire_alu(decoder_out &dec) {
  alu_type alut = std::get<alu_type>(dec.op);
  if (alut == alu_type::JAL || alut == alu_type::JALR) {
    regfile.write(dec.rd, PC + 4);
    PC = alu_out;
    if (dec.rd != 0) {
      std::cout << "writing to " << std::hex << std::to_string(dec.rd)
                << " value(return address): " << alu_out << std::endl;
    } else
      std::cout << "return address is discarded" << std::endl;
  } else {
    regfile.write(dec.rd, alu_out);
    PC = PC + 4;
    std::cout << "writing to " << std::hex << std::to_string(dec.rd)
              << " value: " << alu_out << std::endl;
  }
}
