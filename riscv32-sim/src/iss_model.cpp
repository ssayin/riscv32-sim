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

namespace {
uint32_t do_alu(enum alu opt, uint32_t opd_1, uint32_t opd_2);
bool     should_branch(uint32_t opd_1, uint32_t opd_2, enum branch b_type);
} // namespace

void iss_model::trace(fmt::ostream &out) {
  char buf[128] = {0};
  disasm_inst(buf, sizeof(buf), rv32, static_cast<uint32_t>(pc),
              mem.read_word(static_cast<uint32_t>(pc)));
  out.print("{:>#12x}\t{}\n", static_cast<uint32_t>(pc), buf);
}

void iss_model::step() {
  pc.set(static_cast<uint32_t>(pc) + 4);

  try {
    auto dec = next_op();
    exec(dec);
  } catch (sync_exception &ex) {
    handle(ex);
  }

  pc.update();
}

op iss_model::next_op() {
  auto instr = mem.read_word(static_cast<uint32_t>(pc));
  op   dec   = decode(instr);
  fmt::print("\n{:>#12x}\t{:>#12x}\t", static_cast<uint32_t>(pc), instr);
  return dec;
}

void iss_model::exec(op &dec) {
  switch (dec.tgt) {
  case target::store:
    store(dec);
    break;
  case target::alu:
    handle_alu(dec);
    break;
  case target::load:
    handle_load(dec);
    break;
  case target::branch:
    handle_branch(dec);
    break;
  case target::csr:
    csr(dec);
    break;
  case target::mret:
    handle_mret();
    break;
  case target::ecall:
    dispatch_ecall();
    break;
  case target::illegal:
    throw sync_exception(trap_cause::exp_inst_illegal);
  case target::ebreak:
    throw sync_exception(trap_cause::exp_breakpoint);
  }
}

void iss_model::handle_branch(const op &dec) {
  if (should_branch(regf.read(dec.rs1), regf.read(dec.rs2),
                    std::get<enum branch>(dec.opt))) {
    pc.set(dec.imm + static_cast<uint32_t>(pc));
  }
}

void iss_model::handle_load(op &dec) {
  auto res = load(dec);
  regf.write(dec.rd, res);
}

void iss_model::handle_alu(op &dec) {
  uint32_t opd_1 = dec.use_pc ? static_cast<uint32_t>(pc) : regf.read(dec.rs1);
  uint32_t opd_2 = dec.has_imm ? dec.imm : regf.read(dec.rs2);

  auto res = do_alu(std::get<enum alu>(dec.opt), opd_1, opd_2);
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
}
void iss_model::dispatch_ecall() const {
  switch (mode) {
  case privilege::user:
    throw sync_exception(trap_cause::exp_ecall_from_u_vu_mode);
  case privilege::supervisor:
    throw sync_exception(trap_cause::exp_ecall_from_vs_mode);
  case privilege::hypervisor:
    throw sync_exception(trap_cause::exp_ecall_from_hs_mode);
  case privilege::machine:
    throw sync_exception(trap_cause::exp_ecall_from_m_mode);
  }
}

void iss_model::handle(sync_exception &ex) {
  auto cause = ex.cause();

  fmt::print("{}", cause);

  if (cause == trap_cause::exp_inst_access_fault)
    throw std::runtime_error("fatal");

  assert((to_int(cause) & masks::sign_bit) == 0);

  // all sync exceptions are taken to machine mode for the time being
  mode = privilege::machine;

  csrf.write(csrf.priv_csr(csr::ucause), to_int(cause));
  csrf.write(csrf.priv_csr(csr::utval), 0);

  auto tvec = csrf.read(csrf.priv_csr(csr::utvec));

  pc.set(tvec & masks::tvec::base_addr);

  save_pc(cause);
}

void iss_model::save_pc(const trap_cause &cause) {
  auto epc = static_cast<uint32_t>(pc) & masks::tvec::base_addr;

  switch (cause) {
  case trap_cause::exp_ecall_from_hs_mode:
  case trap_cause::exp_ecall_from_m_mode:
  case trap_cause::exp_ecall_from_vs_mode:
  case trap_cause::exp_ecall_from_u_vu_mode:
    csrf.write(csrf.priv_csr(csr::uepc), epc);
    // syscall handler for testing
    handle_sys_exit();
    break;
  case trap_cause::exp_breakpoint:
    csrf.write(csrf.priv_csr(csr::uepc), epc);
    break;
  default:
    csrf.write(csrf.priv_csr(csr::uepc), (epc + 4) & masks::tvec::base_addr);
    break;
  }
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
 * 93 is sys_exit
 * since there is no supervisor
 * write register a0 to tohost then halt
 */
void iss_model::handle_sys_exit() {
  if (regf.read(17) == 93) {
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

namespace {
bool should_branch(uint32_t opd_1, uint32_t opd_2, enum branch b_type) {
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
  default:
    throw sync_exception(trap_cause::exp_inst_illegal);
  }

  return false;
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
    return static_cast<int32_t>(opd_1) < static_cast<int32_t>(opd_2);

  case alu::_sltu:
    return opd_1 < opd_2;

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
  default:
    throw std::runtime_error("this is not meant to happen");
  }
}
} // namespace
