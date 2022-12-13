#include "iss_model.hpp"
#include "rv32_isn.hpp"
#include "sync_exception.hpp"
#include <fmt/color.h>
#include <fmt/format.h>

iss_model::iss_model(loader l, sparse_memory &mem)
    : mem(mem), tohost_addr{l.symbol("tohost")}, PC{l.entry()}, cf{mode} {

  mem.write_word(tohost_addr, 0);
  fmt::print("PC is set to {:x}\n", static_cast<uint32_t>(PC));
  // Set read-only CSRs

  // TODO: These CSRs may later be loaded during the boot process.
  cf.write(to_int(csr::misa), misa_value);
  cf.write(to_int(csr::sstatus), 0b1 << 8);
  cf.write(to_int(csr::mstatus), 0b11 << 11);
}

void iss_model::trap_setup(trap_cause cause) {
  auto is_fatal = [](trap_cause cause) {
    switch (cause) {
    case trap_cause::exp_inst_access_fault:
      return true;
    default:
      return false;
    }
  };

  uint16_t privilege_base = to_int(mode) << 8;
  assert((to_int(cause) & consts::sign_bit_mask) == 0);

  cf.write(privilege_base | to_int(csr::ucause), to_int(cause));
  cf.write(privilege_base | to_int(csr::utval), 0);

  if (is_fatal(cause))
    throw std::runtime_error("exception is fatal");

  cf.write(privilege_base | to_int(csr::uepc),
           PC & consts::tvec_base_addr_mask);

  auto tvec = cf.read(privilege_base | to_int(csr::utvec));

  PC.set((tvec & consts::tvec_base_addr_mask) + 4);

  /*
   * syscall handler for testing
   */
  switch (cause) {
  case trap_cause::exp_ecall_from_hs_mode:
  case trap_cause::exp_ecall_from_m_mode:
  case trap_cause::exp_ecall_from_vs_mode:
  case trap_cause::exp_ecall_from_u_vu_mode:
    if (rf.read(17) == 93) {
      fmt::print(fg(fmt::color{0xCFDBD5}),
                 "handling ecall, x17 is 93, x10 is {}. x2 is {}\n",
                 static_cast<int32_t>(rf.read(10)),
                 static_cast<int32_t>(rf.read(2)));

      mem.write_word(tohost_addr, rf.read(10));
      _done = true;
    }
    break;
  }

  /* auto is_interrupt = [&cause]() -> bool { return to_int(cause) &
consts::sign_bit_mask; }; enum { direct   = 0, vectored = 1,
};

  if (is_interrupt()){
    uint8_t tvec_type = (tvec & consts::tvec_type_mask);
    if (tvec_type == vectored) {
      PC = (tvec & consts::tvec_base_addr_mask) + 4 * (to_int(cause) &
                                                       consts::msb_zero_mask);
    }
  } else {
    PC = (tvec & consts::tvec_base_addr_mask);
  }
  */
}

void iss_model::step() {
  PC.set(PC + 4);
  uint32_t isn = mem.read_word(PC);
  fmt::print("\n{:>#12x}\t", static_cast<uint32_t>(PC));
  fmt::print("{:>#12x}\t", isn);
  op dec = decode(isn);

  try {
    if (dec.target == pipeline_target::illegal)
      throw sync_exception(trap_cause::exp_inst_illegal);
    if (dec.target == pipeline_target::ebreak) {
      fmt::print("\n\n{}\n\n", dec.rs2);
      throw sync_exception(trap_cause::exp_breakpoint);
    }
    if (dec.target == pipeline_target::ecall) {
      switch (mode) {
      case privilege_level::user:
        throw sync_exception(trap_cause::exp_ecall_from_u_vu_mode);
      case privilege_level::supervisor:
        throw sync_exception(trap_cause::exp_ecall_from_vs_mode);
      case privilege_level::hypervisor:
        throw sync_exception(trap_cause::exp_ecall_from_hs_mode);
      case privilege_level::machine:
        throw sync_exception(trap_cause::exp_ecall_from_m_mode);
      }
    }

    exec(dec);

    switch (dec.target) {
    case pipeline_target::csr:
      csr(dec);
      break;
    case pipeline_target::mem:
      mem_phase(dec);
    case pipeline_target::alu:
    case pipeline_target::branch:
      break;
    case pipeline_target::mret:
      handle_mret();
      break;
    }

    wb_retire_phase(dec);
  }

  catch (sync_exception &ex) {
    fmt::print(fg(fmt::color{0xCFDBD5}), "{} ",
               static_cast<uint32_t>(ex.cause()));
    fmt::print(fg(fmt::color{0xCFDBD5}), " {} ", ex.what());
    trap_setup(ex.cause());
  }

  PC.update();
}

void iss_model::exec(op &dec) {
  switch (dec.target) {
  case pipeline_target::alu:
    exec_alu(dec);
    break;
  case pipeline_target::branch:
    exec_alu_branch(dec);
    break;
  default:
    break;
  }
}

void iss_model::exec_alu(op &dec) {
  uint32_t opd1 = rf.read(dec.rs1);
  uint32_t opd2 = dec.has_imm ? dec.imm : rf.read(dec.rs2);

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
    alu_out = static_cast<int32_t>(opd1) >> opd2;
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
  uint32_t opd1 = rf.read(dec.rs1);
  uint32_t opd2 = rf.read(dec.rs2);

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

void iss_model::mem_phase(op &dec) {
  if (dec.target != pipeline_target::mem)
    return;

  alu_out = rf.read(dec.rs1) + dec.imm; /* mem addr for load/store */

  switch (std::get<mem_type>(dec.opt)) {
    using enum mem_type;
  case lb:
    mem_out = (static_cast<int32_t>(mem.read_byte(alu_out)) << 24) >> 24;
    break;
  case lh:
    mem_out = (static_cast<int32_t>(mem.read_half(alu_out)) << 16) >> 16;
    break;
  case lw:
    mem_out = mem.read_word(alu_out);
    break;
  case lbu:
    mem_out = mem.read_byte(alu_out);
    break;
  case lhu:
    mem_out = mem.read_half(alu_out);
    break;
  case sb:
    mem.write_byte(alu_out, rf.read(dec.rs2));
    break;
  case sh:
    mem.write_half(alu_out, rf.read(dec.rs2));
    break;
  case sw:
    mem.write_word(alu_out, rf.read(dec.rs2));
    break;
  }

  _done = (alu_out == tohost_addr);
}

void iss_model::wb_retire_phase(op &dec) {
  switch (dec.target) {
  case pipeline_target::mem:
    wb_retire_ls(dec);
    break;
  case pipeline_target::alu:
    wb_retire_alu(dec);
    break;
  case pipeline_target::branch:
    PC.set(PC + (alu_out ? dec.imm : 4));
    break;
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
    rf.write(dec.rd, mem_out);
    break;
  default:
    break;
  }
}

void iss_model::wb_retire_alu(op &dec) {
  switch (std::get<alu_type>(dec.opt)) {
  case alu_type::_jal:
  case alu_type::_jalr:
    rf.write(dec.rd, PC + 4);
    PC.set(alu_out);
    break;
  default:
    rf.write(dec.rd, alu_out);
    break;
  }
}

void iss_model::csr(op &dec) {
  // fmt::print(fg(fmt::color{0x242423}), "csr");

  switch (std::get<csr_type>(dec.opt)) {
    using enum csr_type;
  case csrrw: {
    uint32_t tmp = cf.read(dec.imm);
    cf.write(dec.imm, rf.read(dec.rs1));
    rf.write(dec.rd, tmp);
  } break;

  case csrrs: {
    uint32_t tmp = cf.read(dec.imm);
    if (dec.rs1 != 0)
      cf.write(dec.imm, tmp | rf.read(dec.rs1));
    rf.write(dec.rd, tmp);
  } break;

  case csrrc: {
    uint32_t tmp = cf.read(dec.imm);
    if (dec.rs1 != 0)
      cf.write(dec.imm, tmp & (!rf.read(dec.rs1)));
    rf.write(dec.rd, tmp);
  } break;

  case csrrwi: {
    uint32_t tmp = (dec.rd != 0) ? cf.read(dec.imm) : 0;
    rf.write(dec.rd, tmp); // write to x0 is discarded
    cf.write(dec.imm, dec.rs1);
  } break;

  case csrrsi: {
    uint32_t tmp = cf.read(dec.imm);
    cf.write(dec.imm, dec.rs1 | tmp);
    rf.write(dec.rd, tmp);
  } break;

  case csrrci: {
    uint32_t tmp = cf.read(dec.imm);
    cf.write(dec.imm, (!dec.rs1) & tmp);
    rf.write(dec.rd, tmp);
  } break;
  }
}

void iss_model::handle_mret() {
  std::bitset<32> mstat{cf.read(to_int(csr::mstatus))};
  mstat[consts::status_mie] = mstat[consts::status_mpie];
  privilege_level mode_tmp  = static_cast<privilege_level>(
      (mstat[consts::status_mpp + 1] << 1) | mstat[consts::status_mpp]);
  fmt::print(fg(fmt::color{0xE8EDDF}), "mRET ");
  mstat[consts::status_mpie]    = true;
  mstat[consts::status_mpp]     = true;
  mstat[consts::status_mpp + 1] = true;
  cf.write(to_int(csr::mstatus), mstat.to_ulong());
  PC.set(cf.read(to_int(csr::mepc)));
  mode = mode_tmp;

  // fmt::print("\tReturning to {:x}", static_cast<uint32_t>(PC));
}

void iss_model::handle_sret() {
  std::bitset<32> sstat{cf.read(to_int(csr::sstatus))};
  sstat[consts::status_sie] = sstat[consts::status_spie];
  mode                      = static_cast<privilege_level>(
      static_cast<uint8_t>(sstat[consts::status_spp]));
  sstat[consts::status_spie] = false;
  sstat[consts::status_spp]  = false;
  cf.write(to_int(csr::sstatus), sstat.to_ulong());
  PC.set(cf.read(to_int(csr::sepc)));
}
