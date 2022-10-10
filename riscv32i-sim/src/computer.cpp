#include "computer.hpp"
#include "Defs.hpp"
#include "rv32_decode.hpp"

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

void Computer::exec(decoder_out &dec) {
  uint32_t opd1 = regfile.read(dec.rs1);
  uint32_t opd2 = (dec.has_imm & (dec.target != pipeline_type::BRANCH))
                      ? dec.imm
                      : regfile.read(dec.rs2);
  switch (dec.target) {
    using enum pipeline_type;
  case ALU:
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
    break;

  case LS: alu_out = opd1 + opd2; break;

  case BRANCH:
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
    break;
  }
}
