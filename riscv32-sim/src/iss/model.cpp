// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include <bitset>

#include "decoder/decoder.hpp"
#include "iss/arith.hpp"
#include "iss/model.hpp"

namespace iss {
namespace {
uint32_t do_alu(enum alu opt, uint32_t opd_1, uint32_t opd_2);
bool should_branch(uint32_t opd_1, uint32_t opd_2, enum masks::branch b_type);
} // namespace

void model::step() {
  fetch();

  cur_state.pc.set(static_cast<uint32_t>(cur_state.pc) +
                   (cur_state.dec.is_compressed ? 2 : 4));

  if (!is_valid_pc(static_cast<uint32_t>(cur_state.pc))) {
    trap(trap_cause::exp_inst_access_fault);
  }

  exec();
}

void model::commit() {
  cur_state.pc.update();

  // interrupts will not be exported
  interrupt_pending();

  cur_state.csr_staged.clear();
  cur_state.gpr_staged.clear();
}

void model::interrupt_pending() {
  if (opts.mti_enabled &&
      (mem.read64(opts.mtime) > mem.read64(opts.mtimecmp))) {
    set_pending(trap_cause::int_timer_m);
    trap(trap_cause::int_timer_m);
  }
}

void model::fetch() {
  auto x             = mem.read32(static_cast<uint32_t>(cur_state.pc));
  auto is_compressed = [](auto word) { return (word & 0b11) != 0b11; };
  cur_state.dec      = is_compressed(x) ? decode16(x) : decode(x);
  cur_state.instr    = x;
}

void model::exec() {
  switch (cur_state.dec.tgt) {
  case target::store:
    store();
    break;
  case target::alu:
    handle_alu();
    break;
  case target::load:
    handle_load();
    break;
  case target::branch:
    handle_branch();
    break;
  case target::csr:
    csr();
    break;
  case target::mret:
    handle_mret();
    break;
  case target::ecall:
    trap(ecall_cause());
    break;
  case target::illegal:
    trap(trap_cause::exp_inst_illegal);
    break;
  case target::ebreak:
    trap(trap_cause::exp_breakpoint);
    break;
  }
}

void model::handle_branch() {
  if (should_branch(regf.read(cur_state.dec.rs1), regf.read(cur_state.dec.rs2),
                    std::get<enum masks::branch>(cur_state.dec.opt))) {
    auto &pc = cur_state.pc;
    pc.set(cur_state.dec.imm + static_cast<uint32_t>(pc));
  }
}

void model::handle_load() { write(regf, cur_state.dec.rd, load()); }

void model::handle_alu() {
  auto    &pc    = cur_state.pc;
  uint32_t opd_1 = cur_state.dec.use_pc ? static_cast<uint32_t>(pc)
                                        : regf.read(cur_state.dec.rs1);
  uint32_t opd_2 =
      cur_state.dec.has_imm ? cur_state.dec.imm : regf.read(cur_state.dec.rs2);

  auto res = do_alu(std::get<enum alu>(cur_state.dec.opt), opd_1, opd_2);
  switch (std::get<enum alu>(cur_state.dec.opt)) {
  case alu::_jal:
  case alu::_jalr:
    write(regf, cur_state.dec.rd,
          static_cast<uint32_t>(pc) + (cur_state.dec.is_compressed ? 2 : 4));
    pc.set(res);
    break;
  default:
    write(regf, cur_state.dec.rd, res);
    break;
  }
}
trap_cause model::ecall_cause() const {
  switch (mode.mode()) {
  case privilege::user:
    return trap_cause::exp_ecall_from_u_vu_mode;
  case privilege::supervisor:
    return trap_cause::exp_ecall_from_vs_mode;
  case privilege::hypervisor:
    return trap_cause::exp_ecall_from_hs_mode;
  case privilege::machine:
    return trap_cause::exp_ecall_from_m_mode;
  }
  throw unexpected_switch_case_value();
}

void model::trap(trap_cause cause) {
  bool is_interrupt = (to_int(cause) & masks::sign_bit) != 0U;
  if (is_interrupt) {
    std::bitset<32> mstatus{csrf.read(static_cast<uint32_t>(csr::mstatus))};
    auto            i = to_int(cause) & masks::msb_zero;
    std::bitset<32> mie{csrf.read(static_cast<uint32_t>(csr::mie))};
    std::bitset<32> mip{csrf.read(static_cast<uint32_t>(csr::mip))};
    bool take = mie.test(i) && mip.test(i) && mstatus.test(status::mie);

    if (!take) {
      return;
    }

    mip.reset(i);
    write(csrf, static_cast<uint32_t>(csr::mip), mip.to_ulong());

    // TODO: disabled USER mode
    // mode.mode = privilege::machine;

    mstatus[status::mpp]     = true;
    mstatus[status::mpp + 1] = true;

    /* TODO */
    mstatus[status::mpie] = mstatus[status::mie];
    mstatus[status::mie]  = false;

    write(csrf, static_cast<uint32_t>(csr::mstatus), mstatus.to_ulong());
  }

  write(csrf, mode.priv_csr(csr::ucause), to_int(cause));
  write(csrf, mode.priv_csr(csr::utval), 0);

  auto tvec = csrf.read(mode.priv_csr(csr::utvec));

  auto entry = tvec & masks::tvec::base_addr;

  if (is_interrupt) {
    entry = entry + (to_int(cause) & masks::msb_zero) * 4;
  }

  auto &pc = cur_state.pc;
  pc.set(entry);

  save_pc(cause);
  if (is_interrupt) pc.update();
}

void model::save_pc(const trap_cause &cause) {
  auto &pc  = cur_state.pc;
  auto  epc = static_cast<uint32_t>(pc) & masks::tvec::base_addr;

  switch (cause) {
  case trap_cause::exp_ecall_from_hs_mode:
  case trap_cause::exp_ecall_from_m_mode:
  case trap_cause::exp_ecall_from_vs_mode:
  case trap_cause::exp_ecall_from_u_vu_mode:
    write(csrf, mode.priv_csr(csr::uepc), epc);
    handle_sys_exit();
    break;
  case trap_cause::exp_breakpoint:
    write(csrf, mode.priv_csr(csr::uepc), epc);
    break;
  default:
    write(csrf, mode.priv_csr(csr::uepc),
          /*todo*/ (epc + 4) & masks::tvec::base_addr);
    break;
  }
}

void model::csr() {
  const masks::sys &Sys = std::get<masks::sys>(cur_state.dec.opt);
  uint32_t          tmp = 0;
  if (Sys != masks::sys::csrrwi || cur_state.dec.rd != 0) {
    if (mode.can_read(cur_state.dec.imm)) {
      tmp = csrf.read(cur_state.dec.imm);
    } else {
      trap(trap_cause::exp_inst_illegal);
      return;
    }
  }

  if (Sys == masks::sys::csrrw ||
      (Sys == masks::sys::csrrs && cur_state.dec.rs1 != 0) ||
      (Sys == masks::sys::csrrc && cur_state.dec.rs1 != 0)) {

    if (mode.can_read(cur_state.dec.rs1)) {
      uint32_t I = regf.read(cur_state.dec.rs1);

      if (Sys == masks::sys::csrrs) {
        I |= tmp;
      }
      if (Sys == masks::sys::csrrc) {
        I = tmp & !I;
      }

      if (mode.can_write(cur_state.dec.imm)) {
        write(csrf, cur_state.dec.imm, I);
      } else {
        trap(trap_cause::exp_inst_illegal);
        return;
      }
    } else {
      trap(trap_cause::exp_inst_illegal);
      return;
    }
  }

  if (Sys == masks::sys::csrrwi || Sys == masks::sys::csrrsi ||
      Sys == masks::sys::csrrci) {
    uint32_t I = cur_state.dec.rs1;

    if (Sys == masks::sys::csrrsi) {
      I |= tmp;
    }
    if (Sys == masks::sys::csrrci) {
      I = (!I) & tmp;
    }

    if (mode.can_write(cur_state.dec.imm)) {
      write(csrf, cur_state.dec.imm, I);
    } else {
      trap(trap_cause::exp_inst_illegal);
      return;
    }
  }

  write(regf, cur_state.dec.rd, tmp);
}

uint32_t model::load() {

  auto addr = regf.read(cur_state.dec.rs1) +
              cur_state.dec.imm; /* mem addr for load/store */

  switch (std::get<enum masks::load>(cur_state.dec.opt)) {
  case masks::load::lb: {
    return (static_cast<int32_t>(mem.read8(addr)) << 24) >> 24;
  }
  case masks::load::lh: {
    return (static_cast<int32_t>(mem.read16(addr)) << 16) >> 16;
  }
  case masks::load::lw: {
    return mem.read32(addr);
  }
  case masks::load::lbu: {
    return mem.read8(addr);
  }
  case masks::load::lhu: {
    return mem.read16(addr);
  }
  default:
    throw unexpected_switch_case_value();
  }
}

void model::store() {
  auto addr = regf.read(cur_state.dec.rs1) +
              cur_state.dec.imm; /* mem addr for load/store */

  switch (std::get<enum masks::store>(cur_state.dec.opt)) {
  case masks::store::sb:
    mem.write8(addr, regf.read(cur_state.dec.rs2));
    break;
  case masks::store::sh:
    mem.write16(addr, regf.read(cur_state.dec.rs2));
    break;
  case masks::store::sw:
    mem.write32(addr, regf.read(cur_state.dec.rs2));
    break;
  default:
    throw unexpected_switch_case_value();
  }

  is_done = (addr == tohost_addr);
}

/*
 * 93 is sys_exit
 * since there is no supervisor
 * write register a0 to tohost then halt
 */
void model::handle_sys_exit() {
  if (regf.read(17) == 93) {
    mem.write32(tohost_addr, regf.read(10));
    is_done = true;
  }
}

void model::handle_mret() {

  std::bitset<32> mstat{csrf.read(to_int(csr::mstatus))};
  /* The privilege before taking the trap to machine-mode */
  privilege pp =
      static_cast<privilege>((static_cast<int>(mstat[status::mpp + 1]) << 1) |
                             static_cast<int>(mstat[status::mpp]));

  mstat[status::mie]  = mstat[status::mpie];
  mstat[status::mpie] = true;

  /* Set MPP to machine-mode */
  mstat[status::mpp]     = true;
  mstat[status::mpp + 1] = true;

  /* Commit mstatus */
  write(csrf, to_int(csr::mstatus), mstat.to_ulong());

  /* Restore exception program counter */
  auto &pc = cur_state.pc;
  pc.set(csrf.read(to_int(csr::mepc)));

  mode.set_mode(pp);
}

void model::set_pending(trap_cause cause) {
  auto            i = to_int(cause) & masks::msb_zero;
  std::bitset<32> mip{csrf.read(static_cast<uint32_t>(csr::mip))};
  mip.set(i);
  write(csrf, static_cast<uint32_t>(csr::mip), mip.to_ulong());
}

namespace {
bool should_branch(uint32_t opd_1, uint32_t opd_2, enum masks::branch b_type) {
  switch (b_type) {
    using enum masks::branch;
  case beq:
    return opd_1 == opd_2;
  case bne:
    return opd_1 != opd_2;
  case blt:
    return static_cast<int32_t>(opd_1) < static_cast<int32_t>(opd_2);
  case bltu:
    return opd_1 < opd_2;
  case bge:
    return static_cast<int32_t>(opd_1) >= static_cast<int32_t>(opd_2);
  case bgeu:
    return opd_1 >= opd_2;
  default:
    throw model::unexpected_switch_case_value();
  }
}

uint32_t do_alu(enum alu opt, uint32_t opd_1, uint32_t opd_2) {

  auto is_overflow = [](int32_t dividend, int32_t divisor) {
    return (dividend == -2147483648) && (divisor == -1);
  };

  switch (opt) {
  case alu::_or:
    return opd_1 | opd_2;

  case alu::_and:
    return opd_1 & opd_2;

  case alu::_xor:
    return opd_1 ^ opd_2;

  case alu::_add:
  case alu::_auipc:
  case alu::_jal:
    return opd_1 + opd_2;

  case alu::_sub:
    return opd_1 - opd_2;

  case alu::_sll:
    return opd_1 << opd_2;

  case alu::_srl:
    return opd_1 >> opd_2;

  case alu::_sra:
    return static_cast<int32_t>(opd_1) >> opd_2;

  case alu::_slt:
    return static_cast<uint32_t>(static_cast<int32_t>(opd_1) <
                                 static_cast<int32_t>(opd_2));

  case alu::_sltu:
    return static_cast<uint32_t>(opd_1 < opd_2);

  case alu::_jalr:
    return (opd_1 + opd_2) & masks::lsb_zero;

    /*
     * Extension M
     */

  case alu::_mulh:
    return mulh(static_cast<int32_t>(opd_1), static_cast<int32_t>(opd_2));

  case alu::_mulhsu:
    return mulhsu(static_cast<int32_t>(opd_1), opd_2);

  case alu::_mulhu:
    return mulhu(opd_1, opd_2);

  case alu::_mul:
    return static_cast<int32_t>(opd_1) * static_cast<int32_t>(opd_2);

    /*
 clang-format off
+------------------------+-----------+----------+----------+----------+
|       Condition        |  Dividend |  Divisor |  DIVU[W] |  REMU[W] |
+------------------------+-----------+----------+----------+----------+
| Division by zero       |  x        |  0       |  2^L − 1 |  x       |
+------------------------+-----------+----------+----------+----------+
clang-format on
  */

  case alu::_divu:
    if (opd_2 == 0) return std::numeric_limits<uint32_t>::max();
    return opd_1 / opd_2;

  case alu::_remu:
    if (opd_2 == 0) return opd_1;
    return opd_1 % opd_2;

    /*
 clang-format off
+------------------------+-----------+----------+-----------+---------+
|       Condition        |  Dividend |  Divisor |   DIV[W]  |  REM[W] |
+------------------------+-----------+----------+-----------+---------+
| Division by zero       |  x        |  0       |  −1       |  x      |
| Overflow (signed only) |  −2^{L−1} |  −1      |  −2^{L−1} |  0      |
+------------------------+-----------+----------+-----------+---------+
clang-format on
 */

  case alu::_div: {
    auto dividend = static_cast<int32_t>(opd_1);
    auto divisor  = static_cast<int32_t>(opd_2);
    if (divisor == 0) return -1;
    if (is_overflow(dividend, divisor)) return dividend;
    return dividend / divisor;
  }

  case alu::_rem: {
    auto dividend = static_cast<int32_t>(opd_1);
    auto divisor  = static_cast<int32_t>(opd_2);
    if (divisor == 0) return dividend;
    if (is_overflow(dividend, divisor)) return 0;
    return dividend % divisor;
  }
  default:
    throw model::unexpected_switch_case_value();
  }
}

} // namespace
  //
bool model::is_address_initialized(
    const std::vector<std::tuple<uint64_t, uint64_t>> &ranges,
    uint64_t                                           address) {
  return std::any_of(
      ranges.begin(), ranges.end(), [address](const auto &range) {
        return address >= std::get<0>(range) && address <= std::get<1>(range);
      });
}
} // namespace iss
