#include "iss_model.hpp"
#include "rv32_isn.hpp"
#include <fmt/format.h>

uint32_t iss_model::reg_file::read(uint8_t index) {
  assert(index < 32u);
  return x[index];
}

void iss_model::reg_file::write(uint8_t index, uint32_t data) {
  assert(index < 32u);
  if (index == 0)
    return;
  fmt::print("Writing to x{}, value: {:#x}\n", index, data);
  x[index] = data;
}

void iss_model::step() {
  uint32_t isn = mem.read_word(PC);
  fmt::print("PC: {:#x}, Inst: {:#x}\n", PC, isn);
  op dec = decode(isn);

  exec(dec);

  switch (dec.target) {
  case pipeline_target::mem:
    mem_phase(dec);
    break;
  case pipeline_target::alu:
  case pipeline_target::branch:
    break;
  case pipeline_target::csr:
    PC = PC + 4;
    return;
  case pipeline_target::tret:
    return;
  }
  wb_retire_phase(dec);
}

void iss_model::exec(op &dec) {
  switch (dec.target) {
  case pipeline_target::alu:
    exec_alu(dec);
    break;
  case pipeline_target::mem:
    alu_out = regfile.read(dec.rs1) + dec.imm; /* mem addr for load/store */
    break;
  case pipeline_target::branch:
    exec_alu_branch(dec);
    break;
  case pipeline_target::csr:
    csr(dec);
    break;
  case pipeline_target::tret:
    tret(dec);
    break;
  }
}

void iss_model::exec_alu(op &dec) {
  uint32_t opd1 = regfile.read(dec.rs1);
  uint32_t opd2 = dec.has_imm ? dec.imm : regfile.read(dec.rs2);

  switch (std::get<alu_type>(dec.opt)) {
    using enum alu_type;
  case _or:
    alu_out = opd1 | opd2;
    break;
  case _and:
    alu_out = opd1 & opd2;
    break;
  case _xor:
    alu_out = opd1 ^ opd2;
    break;
  case _add:
    alu_out = opd1 + opd2;
    break;
  case _sub:
    alu_out = opd1 - opd2;
    break;
  case _sll:
    alu_out = opd1 << opd2;
    break;
  case _srl:
    alu_out = opd1 >> opd2;
    break;
  case _sra:
    alu_out = sign_extend(opd1, opd2);
    break;
  case _mul:
    alu_out = offset<0u, 31u>(static_cast<uint64_t>(
        static_cast<int64_t>(opd1) * static_cast<int64_t>(opd2)));
    break;
  case _mulh:
    alu_out = offset<32u, 61u>(static_cast<uint64_t>(
        static_cast<int64_t>(opd1) * static_cast<int64_t>(opd2)));
    break;
  case _mulhsu:
    alu_out = offset<32u, 61u>(
        static_cast<uint64_t>(static_cast<int64_t>(opd1) * opd2));
    break;
  case _mulhu:
    alu_out = offset<32u, 61u>(static_cast<uint64_t>(opd1) * opd2);
    break;
  case _div:
    alu_out = static_cast<uint32_t>(static_cast<int32_t>(opd1) /
                                    static_cast<int32_t>(opd2));
    break;
  case _divu:
    alu_out = opd1 / opd2;
    break;
  case _rem:
    alu_out = static_cast<uint32_t>(static_cast<int32_t>(opd1) %
                                    static_cast<int32_t>(opd2));
    break;
  case _remu:
    alu_out = opd1 % opd2;
    break;
  case _slt:
    alu_out = static_cast<int32_t>(opd1) < static_cast<int32_t>(opd2);
    break;
  case _sltu:
    alu_out = opd1 < opd2;
    break;
  case AUIPC:
  case JAL:
    alu_out = PC + opd2;
    break;
  case JALR:
    alu_out = (opd1 + opd2) & 0xFFFFFFFE;
    break;
  }
}

void iss_model::exec_alu_branch(op &dec) {
  uint32_t opd1 = regfile.read(dec.rs1);
  uint32_t opd2 = regfile.read(dec.rs2);

  switch (std::get<branch_type>(dec.opt)) {
    using enum branch_type;
  case beq:
    alu_out = opd1 == opd2;
    break;
  case bne:
    alu_out = opd1 != opd2;
    break;
  case blt:
    alu_out = static_cast<int32_t>(opd1) < static_cast<int32_t>(opd2);
    break;
  case bltu:
    alu_out = opd1 < opd2;
    break;
  case bge:
    alu_out = static_cast<int32_t>(opd1) >= static_cast<int32_t>(opd2);
    break;
  case bgeu:
    alu_out = opd1 >= opd2;
    break;
  }
}

uint32_t sign_extend(uint32_t in, uint8_t shamt) {
  return static_cast<uint32_t>(static_cast<int32_t>(in << shamt) >> shamt);
}

void iss_model::mem_phase(op &dec) {
  if (dec.target != pipeline_target::mem)
    return;
  terminate = (alu_out == tohost_addr);

  switch (std::get<mem_type>(dec.opt)) {
    using enum mem_type;
  case lb:
    mem_out = sign_extend(mem.read_byte(alu_out), 24);
    break;
  case lh:
    mem_out = sign_extend(mem.read_half(alu_out), 16);
    break;
  case lw:
    mem_out = mem.read_word(alu_out);
    break;
  case lbu:
    mem_out = mem.read_byte(alu_out) << 24;
    break;
  case lhu:
    mem_out = mem.read_half(alu_out) << 16;
    break;
  case sb:
    mem.write_byte(alu_out, regfile.read(dec.rs2));
    break;
  case sh:
    mem.write_half(alu_out, regfile.read(dec.rs2));
    break;
  case sw:
    mem.write_word(alu_out, regfile.read(dec.rs2));
    break;
  }
}

void iss_model::wb_retire_phase(op &dec) {
  switch (dec.target) {
  case pipeline_target::mem:
    wb_retire_ls(dec);
    PC += 4;
    break;
  case pipeline_target::alu:
    wb_retire_alu(dec);
    break;
  case pipeline_target::branch:
    PC = PC + (alu_out ? dec.imm : 4);
    break;
  case pipeline_target::csr: {
    csr(dec);
    break;
  }
  default:
    break;
  }
}

void iss_model::wb_retire_ls(op &dec) {
  switch (std::get<mem_type>(dec.opt)) {
    using enum mem_type;
  case lb:
  case lh:
  case lw:
  case lbu:
  case lhu:
    regfile.write(dec.rd, mem_out);
    break;
  default:
    break;
  }
}

void iss_model::wb_retire_alu(op &dec) {
  if (alu_type alut = std::get<alu_type>(dec.opt);
      alut == alu_type::JAL || alut == alu_type::JALR) {
    regfile.write(dec.rd, PC + 4);
    PC = alu_out;
    if (dec.rd != 0) {
      fmt::print("JUMP to {:#x}: Return Addr: {:#x}\n", PC, alu_out);
    } else
      fmt::print("JUMP to {:#x}: Return Addr is discarded\n", PC);
  } else {
    regfile.write(dec.rd, alu_out);
    PC = PC + 4;
  }
}

void iss_model::csr(op &dec) {
  switch (std::get<csr_type>(dec.opt)) {
    using enum csr_type;
  case csrrw: {
    uint32_t tmp = csrh.read(dec.imm);
    csrh.write(dec.imm, regfile.read(dec.rs1));
    regfile.write(dec.rd, tmp);
  } break;

  case csrrs: {
    uint32_t tmp = csrh.read(dec.imm);
    csrh.write(dec.imm, tmp | regfile.read(dec.rs1));
    regfile.write(dec.rd, tmp);
  } break;

  case csrrc: {
    uint32_t tmp = csrh.read(dec.imm);
    csrh.write(dec.imm, tmp & (!regfile.read(dec.rs1)));
    regfile.write(dec.rd, tmp);
  } break;

  case csrrwi: {
    regfile.write(dec.rd, csrh.read(dec.imm));
    csrh.write(dec.imm, dec.rs1);
  } break;

  case csrrsi: {
    uint32_t tmp = csrh.read(dec.imm);
    csrh.write(dec.imm, dec.rs1 | tmp);
    regfile.write(dec.rd, tmp);

  } break;

  case csrrci: {
    uint32_t tmp = csrh.read(dec.imm);
    csrh.write(dec.imm, (!dec.rs1) & tmp);
    regfile.write(dec.rd, tmp);
  } break;
  }
}

void iss_model::handle_mret() {
  /*
   * y <- MPP
   * MIE <- MPIE
   * mode <- y
   * MPIE <- 1
   * MPP <- U (if impl.), M
   */
  uint32_t stat = csrh.read(csr::mstatus);
  uint8_t  y    = offset<11u, 12u>(stat);
  uint8_t  mpie = offset<7u, 7u>(stat);
  stat          = stat & (0xFFFFFF8 | ((mpie) << 3)); // make sure its 3
  csrh.mode     = static_cast<csr_handler::priv>(y);  // recheck
  stat          = stat | (1 << 7);
  // might add user mode later
  stat = stat & (0xFFFFE7FF | (csr_handler::priv::machine << 11));
  csrh.write(csr::mstatus, stat);
  PC = csrh.read(csr::mepc);
}

void iss_model::handle_sret() {
  /*
   * y <- SPP
   * SIE <- SPIE
   * mode <- y
   * SPIE <- 1
   * SPP <- U (if impl.), M
   */
  uint32_t stat = csrh.read(csr::sstatus);
  uint8_t  y    = offset<8u, 8u>(stat);
  uint8_t  spie = offset<5u, 5u>(stat);

  stat = stat & (0xFFFFFFC | ((spie) << 1));

  csrh.mode = static_cast<csr_handler::priv>(y); // recheck

  stat = stat | (1 << 5);

  // might add user mode later
  stat = stat & (0xFFFFFEFF | csr_handler::priv::machine); // spp [8:8]

  csrh.write(csr::sstatus, stat);
  PC = csrh.read(csr::sepc);
}

void iss_model::tret(op &op) {
  switch (std::get<trap_ret_type>(op.opt)) {
    using enum trap_ret_type;
  case machine:
    handle_mret();
    break;
  case supervisor:
    handle_sret();
    break;
  case user:
    break;
  }
}
