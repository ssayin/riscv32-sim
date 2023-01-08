#include "iss_model.hpp"
#include "arith.hpp"
#include "decoder/decoder.hpp"
#include "instr/rv32_isn.hpp"
#include "memory/sparse_memory.hpp"
#include "zicsr/csr.hpp"
#include "zicsr/privilege.hpp"
#include "zicsr/sync_exception.hpp"
#include "zicsr/trap_cause.hpp"
#include <fmt/color.h>
extern "C" {
#include <riscv-disas.h>
}
#include <stdexcept>

static bool should_branch(uint32_t opd_1, uint32_t opd_2, enum branch b_type) {
  switch (b_type) {
    using enum branch;
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
  }

  return false;
}

iss_model::iss_model(loader l, sparse_memory &mem)
    : tohost_addr{l.symbol("tohost")}, mem{mem}, pc{l.entry()},
      mode{privilege::machine}, csrf(mode), is_done{false} {}

void iss_model::step() {
  pc.set(static_cast<uint32_t>(pc) + 4);
  auto instr = mem.read_word(static_cast<uint32_t>(pc));
  op   dec   = decode(instr);
  fmt::print("\n{:>#12x}\t{:>#12x}\t", static_cast<uint32_t>(pc), instr);

  char buf[128] = {0};
  disasm_inst(buf, sizeof(buf), rv32, static_cast<uint32_t>(pc), instr);

  try {
    if (dec.tgt == target::ecall) {
      switch (mode) {
      case privilege::user:
        throw sync_exception(trap_cause::exp_ecall_from_u_vu_mode);
      case privilege::supervisor:
        break;
        // throw sync_exception(trap_cause::exp_ecall_from_vs_mode);
      case privilege::hypervisor:
        break;
        // throw sync_exception(trap_cause::exp_ecall_from_hs_mode);
      case privilege::machine:
        throw sync_exception(trap_cause::exp_ecall_from_m_mode);
      }

      // throw sync_exception(trap_cause::exp_ecall_from_m_mode);
    }

    switch (dec.tgt) {
    case target::store:
      store(dec);
      break;
    case target::alu: {
      auto res = alu(dec);
      switch (std::get<enum alu>(dec.opt)) {
      case alu::_jal:
      case alu::_jalr:
        regf.write(dec.rd, static_cast<uint32_t>(pc) + 4);
        pc.set(res);
        break;
      default:
        regf.write(dec.rd, res);
        break;
      }
    } break;
    case target::load: {
      auto res = load(dec);
      regf.write(dec.rd, res);
    } break;
    case target::branch:
      if (should_branch(regf.read(dec.rs1), regf.read(dec.rs2),
                        std::get<enum branch>(dec.opt))) {
        pc.set(dec.imm + static_cast<uint32_t>(pc));
      }
      break;
    case target::csr:
      csr(dec);
      break;
    case target::mret:
      handle_mret();
      break;
    case target::illegal:
      throw sync_exception(trap_cause::exp_inst_illegal);
    case target::ebreak:
      throw sync_exception(trap_cause::exp_breakpoint);
    case target::ecall:
      break;
    }
  } catch (sync_exception &ex) {
    handle_sync_exception(ex);
  }

  pc.update();
}
void iss_model::handle_sync_exception(sync_exception &ex) {
  auto cause = ex.cause();

  fmt::print("{}", cause);

  if (cause == trap_cause::exp_inst_access_fault)
    throw std::runtime_error("fatal");

  assert((to_int(cause) & masks::sign_bit) == 0);

  // all sync exceptions are taken to machine mode for the time being
  mode = privilege::machine;

  csrf.write(priv_csr(csr::ucause), to_int(cause));
  csrf.write(priv_csr(csr::utval), 0);
  auto tvec = csrf.read(priv_csr(csr::utvec));

  pc.set(tvec & masks::tvec::base_addr);

  auto epc = static_cast<uint32_t>(pc) & masks::tvec::base_addr;

  switch (cause) {
  case trap_cause::exp_ecall_from_hs_mode:
  case trap_cause::exp_ecall_from_m_mode:
  case trap_cause::exp_ecall_from_vs_mode:
  case trap_cause::exp_ecall_from_u_vu_mode:
    csrf.write(priv_csr(csr::uepc), epc);
    // syscall handler for testing
    handle_sys_exit();
    break;
  case trap_cause::exp_breakpoint:
    csrf.write(priv_csr(csr::uepc), epc);
    break;
  default:
    csrf.write(priv_csr(csr::uepc), (epc + 4) & masks::tvec::base_addr);
    break;
  }
}

uint32_t iss_model::alu(op &dec) {

  uint32_t opd_1 = regf.read(dec.rs1);

  uint32_t opd_2 = dec.has_imm ? dec.imm : regf.read(dec.rs2);

  switch (std::get<enum alu>(dec.opt)) {

  case alu::_or:
    return opd_1 | opd_2;

  case alu::_and:
    return opd_1 & opd_2;

  case alu::_xor:
    return opd_1 ^ opd_2;

  case alu::_add:
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
    return static_cast<int32_t>(opd_1) < static_cast<int32_t>(opd_2);
  case alu::_sltu:
    return opd_1 < opd_2;

  case alu::_auipc:
  case alu::_jal:
    return static_cast<uint32_t>(pc) + opd_2;

  case alu::_jalr:
    return (opd_1 + opd_2) & 0xFFFFFFFE;

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
    auto dividend = static_cast<int32_t>(opd_1);
    auto divisor  = static_cast<int32_t>(opd_2);
    if (divisor == 0) return -1;
    if ((dividend == -2147483648) && (divisor == -1)) return dividend;
    return dividend / divisor;
  }

  case alu::_rem: {
    auto dividend = static_cast<int32_t>(opd_1);
    auto divisor  = static_cast<int32_t>(opd_2);
    if (divisor == 0) return dividend;
    if ((dividend == -2147483648) && (divisor == -1)) return 0;
    return dividend % divisor;
  }
  }
  throw std::runtime_error("this is not meant to happen");
}

void iss_model::csr(op &dec) {
  switch (std::get<sys>(dec.opt)) {
    using enum sys;
  case csrrw: {
    uint32_t tmp = csrf.read(dec.imm);
    csrf.write(dec.imm, regf.read(dec.rs1));
    regf.write(dec.rd, tmp);
  } break;

  case csrrs: {
    uint32_t tmp = csrf.read(dec.imm);
    if (dec.rs1 != 0) csrf.write(dec.imm, tmp | regf.read(dec.rs1));
    regf.write(dec.rd, tmp);
  } break;

  case csrrc: {
    uint32_t tmp = csrf.read(dec.imm);
    if (dec.rs1 != 0) csrf.write(dec.imm, tmp & (!regf.read(dec.rs1)));
    regf.write(dec.rd, tmp);
  } break;

  case csrrwi: {
    uint32_t tmp = (dec.rd != 0) ? csrf.read(dec.imm) : 0;
    regf.write(dec.rd, tmp); // write to x0 is discarded
    csrf.write(dec.imm, dec.rs1);
  } break;

  case csrrsi: {
    uint32_t tmp = csrf.read(dec.imm);
    csrf.write(dec.imm, dec.rs1 | tmp);
    regf.write(dec.rd, tmp);
  } break;

  case csrrci: {
    uint32_t tmp = csrf.read(dec.imm);
    csrf.write(dec.imm, (!dec.rs1) & tmp);
    regf.write(dec.rd, tmp);
  } break;

  case other:
    break;
  }
}

uint32_t iss_model::load(op &dec) {

  auto addr = regf.read(dec.rs1) + dec.imm; /* mem addr for load/store */

  switch (std::get<enum load>(dec.opt)) {
  case load::lb:
    return (static_cast<int32_t>(mem.read_byte(addr)) << 24) >> 24;
  case load::lh:
    return (static_cast<int32_t>(mem.read_half(addr)) << 16) >> 16;
  case load::lw:
    return mem.read_word(addr);
  case load::lbu:
    return mem.read_byte(addr);
  case load::lhu:
    return mem.read_half(addr);
  default:
    throw sync_exception(trap_cause::exp_inst_illegal);
  }
}

void iss_model::store(op &dec) {
  auto addr = regf.read(dec.rs1) + dec.imm; /* mem addr for load/store */

  switch (std::get<enum store>(dec.opt)) {
  case store::sb:
    mem.write_byte(addr, regf.read(dec.rs2));
    break;
  case store::sh:
    mem.write_half(addr, regf.read(dec.rs2));
    break;
  case store::sw:
    mem.write_word(addr, regf.read(dec.rs2));
    break;
  default:
    throw sync_exception(trap_cause::exp_inst_illegal);
  }

  is_done = (addr == tohost_addr);
}

/*
 * syscalls are __naked__ functions that do not return
 * all changes to the registers are persistent
 *
 * followed by ecall
 */
void iss_model::handle_sys_exit() {
  // 93 is sys_exit
  if (regf.read(17) == 93) {
    // since there is no supervisor
    // write to tohost_addr then halt
    mem.write_word(tohost_addr, regf.read(10));
    is_done = true;
    fmt::print("\n");
  }
}

void iss_model::handle_mret() {
  std::bitset<32> mstat{csrf.read(to_int(csr::mstatus))};
  mstat[status::mie] = mstat[status::mpie];
  privilege mode_tmp = static_cast<privilege>((mstat[status::mpp + 1] << 1) |
                                              mstat[status::mpp]);
  fmt::print(fg(fmt::color{0xE8EDDF}), "mRET ");
  mstat[status::mpie]    = true;
  mstat[status::mpp]     = false;
  mstat[status::mpp + 1] = false;
  csrf.write(to_int(csr::mstatus), mstat.to_ulong());
  pc.set(csrf.read(to_int(csr::mepc)));
  mode = mode_tmp;
}

void iss_model::handle_sret() {
  std::bitset<32> sstat{csrf.read(to_int(csr::sstatus))};
  sstat[status::sie] = sstat[status::spie];
  mode = static_cast<privilege>(static_cast<uint8_t>(sstat[status::spp]));
  sstat[status::spie] = false;
  sstat[status::spp]  = false;
  csrf.write(to_int(csr::sstatus), sstat.to_ulong());
  pc.set(csrf.read(to_int(csr::sepc)));
}
