#include "iss_model.hpp"
#include "arith.hpp"
#include "decoder/decoder.hpp"

#include <bitset>
#include <fmt/color.h>

extern "C" {
#include <riscv-disas.h>
}

namespace {
uint32_t do_alu(enum alu opt, uint32_t opd_1, uint32_t opd_2);
bool should_branch(uint32_t opd_1, uint32_t opd_2, enum masks::branch b_type);
} // namespace

void iss_model::trace_disasm(fmt::ostream &out) {
  std::array<char, 128> buf{};
  disasm_inst(buf.data(), buf.size(), rv32, static_cast<uint32_t>(pc),
              mem.read32(static_cast<uint32_t>(pc)));
  out.print("{:>#12x}\t{}\n", static_cast<uint32_t>(pc), buf.data());
}

void iss_model::step() {
  pc.set(static_cast<uint32_t>(pc) + instr_alignment);
  dec = next_op();
  exec();
  pc.update();

  if (opts.mti_enabled &&
      (mem.read64(opts.mtime) > mem.read64(opts.mtimecmp))) {
    set_pending(trap_cause::int_timer_m);
    trap(trap_cause::int_timer_m);
  }
}

op iss_model::next_op() {
  op dec;
  instr = mem.read32(static_cast<uint32_t>(pc));
  dec   = decode(instr);
  fmt::print("\n{:>#12x}\t{:>#12x}\t", static_cast<uint32_t>(pc), instr);

  return dec;
}

void iss_model::exec() {
  switch (dec.tgt) {
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
    fmt::print(fg(fmt::color{0xE8EDDF}), "mRET ");
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

void iss_model::handle_branch() {
  if (should_branch(regf.read(dec.rs1), regf.read(dec.rs2),
                    std::get<enum masks::branch>(dec.opt))) {
    pc.set(dec.imm + static_cast<uint32_t>(pc));
  }
}

void iss_model::handle_load() {
  auto res = load();
  regf.write(dec.rd, res);
}

void iss_model::handle_alu() {
  uint32_t opd_1 = dec.use_pc ? static_cast<uint32_t>(pc) : regf.read(dec.rs1);
  uint32_t opd_2 = dec.has_imm ? dec.imm : regf.read(dec.rs2);

  auto res = do_alu(std::get<enum alu>(dec.opt), opd_1, opd_2);
  switch (std::get<enum alu>(dec.opt)) {
  case alu::_jal:
  case alu::_jalr:
    regf.write(dec.rd, static_cast<uint32_t>(pc) + instr_alignment);
    pc.set(res);
    break;
  default:
    regf.write(dec.rd, res);
    break;
  }
}
trap_cause iss_model::ecall_cause() const {
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
  throw std::runtime_error("");
}

void iss_model::trap(trap_cause cause) {
  if (cause == trap_cause::exp_inst_access_fault) std::exit(EXIT_FAILURE);

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
    csrf.write(static_cast<uint32_t>(csr::mip), mip.to_ulong());

    // TODO: disabled USER mode
    // mode.mode = privilege::machine;

    mstatus[status::mpp]     = true;
    mstatus[status::mpp + 1] = true;

    /* TODO */
    mstatus[status::mpie] = mstatus[status::mie];
    mstatus[status::mie]  = false;

    csrf.write(static_cast<uint32_t>(csr::mstatus), mstatus.to_ulong());
  }

  fmt::print("{}", cause);

  csrf.write(mode.priv_csr(csr::ucause), to_int(cause));
  csrf.write(mode.priv_csr(csr::utval), 0);

  auto tvec = csrf.read(mode.priv_csr(csr::utvec));

  auto entry = tvec & masks::tvec::base_addr;

  if (is_interrupt) {
    entry = entry + (to_int(cause) & masks::msb_zero) * 4;
  }
  pc.set(entry);

  save_pc(cause);
  if (is_interrupt) pc.update();
}

void iss_model::save_pc(const trap_cause &cause) {
  auto epc = static_cast<uint32_t>(pc) & masks::tvec::base_addr;

  switch (cause) {
  case trap_cause::exp_ecall_from_hs_mode:
  case trap_cause::exp_ecall_from_m_mode:
  case trap_cause::exp_ecall_from_vs_mode:
  case trap_cause::exp_ecall_from_u_vu_mode:
    csrf.write(mode.priv_csr(csr::uepc), epc);
    handle_sys_exit();
    break;
  case trap_cause::exp_breakpoint:
    csrf.write(mode.priv_csr(csr::uepc), epc);
    break;
  default:
    csrf.write(mode.priv_csr(csr::uepc),
               (epc + instr_alignment) & masks::tvec::base_addr);
    break;
  }
}

void iss_model::csr() {
  const masks::sys &Sys = std::get<masks::sys>(dec.opt);
  uint32_t          tmp = 0;
  if (Sys != masks::sys::csrrwi || dec.rd != 0) {
    if (mode.can_read(dec.imm)) {
      tmp = csrf.read(dec.imm);
    } else {
      trap(trap_cause::exp_inst_illegal);
      return;
    }
  }

  if (Sys == masks::sys::csrrw || (Sys == masks::sys::csrrs && dec.rs1 != 0) ||
      (Sys == masks::sys::csrrc && dec.rs1 != 0)) {

    if (mode.can_read(dec.rs1)) {
      uint32_t I = regf.read(dec.rs1);

      if (Sys == masks::sys::csrrs) {
        I |= tmp;
      }
      if (Sys == masks::sys::csrrc) {
        I = tmp & !I;
      }

      if (mode.can_write(dec.imm)) {
        csrf.write(dec.imm, I);
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
    uint32_t I = dec.rs1;

    if (Sys == masks::sys::csrrsi) {
      I |= tmp;
    }
    if (Sys == masks::sys::csrrci) {
      I = (!I) & tmp;
    }

    if (mode.can_write(dec.imm)) {
      csrf.write(dec.imm, I);
    } else {
      trap(trap_cause::exp_inst_illegal);
      return;
    }
  }

  regf.write(dec.rd, tmp);
}

uint32_t iss_model::load() {

  auto addr = regf.read(dec.rs1) + dec.imm; /* mem addr for load/store */

  switch (std::get<enum masks::load>(dec.opt)) {
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
    throw std::runtime_error(""); // illegal
  }
}

void iss_model::store() {
  auto addr = regf.read(dec.rs1) + dec.imm; /* mem addr for load/store */

  switch (std::get<enum masks::store>(dec.opt)) {
  case masks::store::sb:
    mem.write8(addr, regf.read(dec.rs2));
    break;
  case masks::store::sh:
    mem.write16(addr, regf.read(dec.rs2));
    break;
  case masks::store::sw:
    mem.write32(addr, regf.read(dec.rs2));
    break;
  default:
    throw std::runtime_error(""); // illegal
  }

  is_done = (addr == tohost_addr);
}

/*
 * 93 is sys_exit
 * since there is no supervisor
 * write register a0 to tohost then halt
 */
void iss_model::handle_sys_exit() {
  if (regf.read(17) == 93) {
    mem.write32(tohost_addr, regf.read(10));
    is_done = true;
    fmt::print("\n");
  }
}

void iss_model::handle_mret() {

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
  csrf.write(to_int(csr::mstatus), mstat.to_ulong());

  /* Restore exception program counter */
  pc.set(csrf.read(to_int(csr::mepc)));

  mode.set_mode(pp);
}

void iss_model::set_pending(trap_cause cause) {
  auto            i = to_int(cause) & masks::msb_zero;
  std::bitset<32> mip{csrf.read(static_cast<uint32_t>(csr::mip))};
  mip.set(i);
  csrf.write(static_cast<uint32_t>(csr::mip), mip.to_ulong());
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
    throw std::runtime_error(""); // illegal
  }

  return false;
}

bool is_overflow(int32_t dividend, int32_t divisor) {
  return (dividend == -2147483648) && (divisor == -1);
}

uint32_t do_alu(enum alu opt, uint32_t opd_1, uint32_t opd_2) {
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
    return mulh(opd_1, opd_2);

  case alu::_mulhsu:
    return mulhsu(opd_1, opd_2);

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
    int32_t dividend = static_cast<int32_t>(opd_1);
    int32_t divisor  = static_cast<int32_t>(opd_2);
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
    throw std::runtime_error("this is not meant to happen");
  }
}
} // namespace
uint8_t address_router::read8(uint32_t off) const {
  if (off >= mtime_addr && off < mtime_addr + 8) {
    return mtime.at(off - mtime_addr).load();
  }
  if (off >= mtimecmp_addr && off < mtimecmp_addr + 8) {
    return mtimecmp.at(off - mtimecmp_addr).load();
  }
  return sparse_memory_accessor::read8(off);
}

void address_router::write8(uint32_t off, uint8_t b) {
  if (off >= mtime_addr && off < mtime_addr + 8) {
    mtime.at(off - mtime_addr).store(b);
  } else if (off >= mtimecmp_addr && off < mtimecmp_addr + 8) {
    mtimecmp.at(off - mtimecmp_addr).store(b);
  } else
    sparse_memory_accessor::write8(off, b);
}

address_router::address_router(sparse_memory &mem, uint32_t mtime_addr,
                               uint32_t mtimecmp_addr)
    : sparse_memory_accessor{mem}, mtime_addr{mtime_addr}, mtimecmp_addr{
                                                               mtimecmp_addr} {}
