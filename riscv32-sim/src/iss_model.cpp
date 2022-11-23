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

  if(trap) goto __trap;

  switch (dec.target) {
  case pipeline_target::mem:
    mem_phase(dec);
    if(trap) goto __trap;
    break;
  case pipeline_target::alu:
  case pipeline_target::branch:
    break;
  case pipeline_target::csr:
    if(trap) goto __trap;
    PC = PC + 4;
    return;
  case pipeline_target::tret:
    tret(dec);
    return;
  }

  wb_retire_phase(dec);
  if(trap) goto __trap;

  return;

  __trap:
    handle_trap();
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
  case _auipc:
  case _jal:
    alu_out = PC + opd2;
    break;
  case _jalr:
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
      alut == alu_type::_jal || alut == alu_type::_jalr) {
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
    uint32_t tmp = read_csr(dec.imm);
    write_csr(dec.imm, regfile.read(dec.rs1));
    regfile.write(dec.rd, tmp);
  } break;

  case csrrs: {
    uint32_t tmp = read_csr(dec.imm);
    write_csr(dec.imm, tmp | regfile.read(dec.rs1));
    regfile.write(dec.rd, tmp);
  } break;

  case csrrc: {
    uint32_t tmp = read_csr(dec.imm);
    write_csr(dec.imm, tmp & (!regfile.read(dec.rs1)));
    regfile.write(dec.rd, tmp);
  } break;

  case csrrwi: {
    regfile.write(dec.rd, read_csr(dec.imm));
    write_csr(dec.imm, dec.rs1);
  } break;

  case csrrsi: {
    uint32_t tmp = read_csr(dec.imm);
    write_csr(dec.imm, dec.rs1 | tmp);
    regfile.write(dec.rd, tmp);
  } break;

  case csrrci: {
    uint32_t tmp = read_csr(dec.imm);
    write_csr(dec.imm, (!dec.rs1) & tmp);
    regfile.write(dec.rd, tmp);
  } break;
  }
}

/*
 An MRET or SRET instruction is used to return from a trap_setup in M-mode or S-mode
respectively. When executing an xRET instruction, supposing xPP holds the value
y, xIE is set to xPIE; the privilege mode is changed to y; xPIE is set to 1; and
xPP is set to the least-privileged supported mode (U if U-mode is implemented,
else M). If xPP̸=M, xRET also sets MPRV=0.
 */

void iss_model::handle_mret() {
  enum {
    MIE  = 3,
    MPIE = 7,
    MPP  = 11, //[11:12]
    MPRV = 17,
  };

  std::bitset<32> mstat{read_csr(csr::mstatus)};
  mstat[MIE] = mstat[MPIE];
  mode  = static_cast<privilege_level>((mstat[MPP + 1] << 1) |
                                                        mstat[MPP]);
  fmt::print("MRET: Changed privilege mode to: {}\n", static_cast<uint8_t>(mode));
  mstat[MPIE]    = true;
  mstat[MPP]     = true;
  mstat[MPP + 1] = true;
  write_csr(csr::mstatus, mstat.to_ulong());
  PC = read_csr(csr::mepc) + 4;

  fmt::print("MRET: returning to addr {}\n", PC);
}

void iss_model::handle_sret() {
  enum {
    SIE  = 1,
    SPIE = 5,
    SPP  = 8,
  };

  std::bitset<32> sstat{read_csr(csr::sstatus)};
  sstat[SIE] = sstat[SPIE];
  mode  = static_cast<privilege_level>(
      static_cast<uint8_t>(sstat[SPP]));
  fmt::print("SRET: Changed privilege mode to: {}\n", static_cast<uint8_t>(mode));
  sstat[SPIE] = true;
  sstat[SPP]  = true;
  write_csr(csr::sstatus, sstat.to_ulong());
  PC = read_csr(csr::sepc) + 4;

  fmt::print("SRET: returning to addr {}\n", PC);
}

void iss_model::tret(op &op) {
  switch (std::get<trap_ret_type>(op.opt)) {
    using enum trap_ret_type;
  case mret:
    handle_mret();
    break;
  case sret:
    handle_sret();
    break;
  default:
    break;
  }
}


inline constexpr uint8_t priv(uint32_t addr) {
  return offset<8u,9u>(addr);
};

void iss_model::write_csr(uint32_t addr, uint32_t v) {
  auto is_readonly = [](uint32_t addr) {
    return offset<10u,11u>(addr) == 0b11;
  };

  if (is_readonly(addr) || (priv(addr) > to_int(mode))) {
    trap_setup(trap_cause::exp_inst_illegal);
    return;
  }

  csrs[addr] = v;
}

uint32_t iss_model::read_csr(uint32_t addr) {
  if (priv(addr) > to_int(mode)) {
    trap_setup(trap_cause::exp_inst_illegal);
    return 0x0;
  }

  return csrs[addr];
}

void iss_model::trap_setup(trap_cause cause) {
  auto cause_csr = [](){
    return csr::mcause;
  };

  auto is_fatal = [](trap_cause cause) { return false; };

  write_csr(cause_csr(), static_cast<uint32_t>(cause));
  write_csr(csr::mtval, 0);
  write_csr(csr::mepc, PC);
  if(is_fatal(cause)) throw std::exception();

  trap = true;

}

void iss_model::handle_trap() {
  auto cause = read_csr(csr::mcause);
  auto tval = read_csr(csr::mtval);
  auto tvec = read_csr(csr::mtvec);

  enum {
    direct = 0,
    vectored = 1,
  };

  if(is_interrupt(static_cast<trap_cause>(cause))) {
    uint8_t tvec_type = (tvec & 0x00000002);
  } else {
    PC = (tvec & 0xFFFFFFFFC);
  }

  fmt::print("Trap Handled\n");

  trap = false;
}