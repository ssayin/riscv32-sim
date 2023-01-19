#include "iss_model.hpp"
#include "arith.hpp"
#include "decoder/decoder.hpp"
#include "instr/rv32_isn.hpp"
#include "memory/sparse_memory.hpp"
#include "zicsr/csr.hpp"
#include "zicsr/privilege.hpp"
#include "zicsr/trap_cause.hpp"
#include <fmt/color.h>
extern "C" {
#include <riscv-disas.h>
}
#include <cstdint>
#include <stdexcept>

namespace {
uint32_t do_alu(enum alu opt, uint32_t opd_1, uint32_t opd_2);
bool     should_branch(uint32_t opd_1, uint32_t opd_2, enum branch b_type);
} // namespace

void iss_model::trace(fmt::ostream &out) {
  std::array<char, 128> buf{};
  disasm_inst(buf.data(), buf.size(), rv32, static_cast<uint32_t>(pc),
              mem.read_word(static_cast<uint32_t>(pc)));
  out.print("{:>#12x}\t{}\n", static_cast<uint32_t>(pc), buf.data());
}

void iss_model::step() {
  pc.set(static_cast<uint32_t>(pc) + instr_alignment);
  op dec = next_op();
  exec(dec);
  pc.update();
}

op iss_model::next_op() {
  op   dec;
  auto instr = mem.read_word(static_cast<uint32_t>(pc));
  dec        = decode(instr);
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
    handle(handle_ecall());
    break;
  case target::illegal:
    handle(trap_cause::exp_inst_illegal);
    break;
  case target::ebreak:
    handle(trap_cause::exp_breakpoint);
    break;
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
    regf.write(dec.rd, static_cast<uint32_t>(pc) + instr_alignment);
    pc.set(res);
    break;
  default:
    regf.write(dec.rd, res);
    break;
  }
}
trap_cause iss_model::handle_ecall() const {
  switch (mode.mode) {
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

void iss_model::handle(trap_cause cause) {


  if (cause == trap_cause::exp_inst_access_fault)
    throw std::runtime_error("fatal");

  bool is_interrupt = (to_int(cause) & masks::sign_bit) != 0U;
  if(is_interrupt) {
    auto status = csrf.read(static_cast<uint32_t>(csr::mie));
    if(!status) return;
    csrf.write(static_cast<uint32_t>(csr::mie), 0);
  }

  fmt::print("{}", cause);


  // all sync exceptions are taken to machine mode for the time being
  mode.mode = privilege::machine;

  csrf.write(mode.priv_csr(csr::ucause), to_int(cause));
  csrf.write(mode.priv_csr(csr::utval), 0);

  auto tvec = csrf.read(mode.priv_csr(csr::utvec));

  auto entry = tvec & masks::tvec::base_addr;


  if (is_interrupt) {
    entry =  entry + (to_int(cause) & masks::msb_zero) * 4;
  }
  pc.set(entry);

  save_pc(cause);
  if(is_interrupt) pc.update();
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

void iss_model::csr(op &dec) {
  const sys &Sys = std::get<sys>(dec.opt);
  uint32_t   tmp = 0;
  if (Sys != sys::csrrwi || dec.rd != 0) {
    if (mode.is_read_legal_csr(dec.imm)) {
      tmp = csrf.read(dec.imm);
    } else {
      handle(trap_cause::exp_inst_illegal);
      return;
    }
  }

  if (Sys == sys::csrrw || (Sys == sys::csrrs && dec.rs1 != 0) ||
      (Sys == sys::csrrc && dec.rs1 != 0)) {

    if (mode.is_read_legal_csr(dec.rs1)) {
      uint32_t I = regf.read(dec.rs1);

      if (Sys == sys::csrrs) {
        I |= tmp;
      }
      if (Sys == sys::csrrc) {
        I = tmp & !I;
      }

      if (mode.is_write_legal_csr(dec.imm)) {
        csrf.write(dec.imm, I);
      } else {
        handle(trap_cause::exp_inst_illegal);
        return;
      }
    } else {
      handle(trap_cause::exp_inst_illegal);
      return;
    }
  }

  if (Sys == sys::csrrwi || Sys == sys::csrrsi || Sys == sys::csrrci) {
    uint32_t I = dec.rs1;

    if (Sys == sys::csrrsi) {
      I |= tmp;
    }
    if (Sys == sys::csrrci) {
      I = (!I) & tmp;
    }

    if (mode.is_write_legal_csr(dec.imm)) {
      csrf.write(dec.imm, I);
    } else {
      handle(trap_cause::exp_inst_illegal);
      return;
    }
  }

  regf.write(dec.rd, tmp);
}

uint32_t iss_model::load(op &dec) {

  auto addr = regf.read(dec.rs1) + dec.imm; /* mem addr for load/store */

  switch (std::get<enum load>(dec.opt)) {
  case load::lb: {
    return (static_cast<int32_t>(mem.read_byte(addr)) << 24) >> 24;
  }
  case load::lh: {
    return (static_cast<int32_t>(mem.read_half(addr)) << 16) >> 16;
  }
  case load::lw: {
    return mem.read_word(addr);
  }
  case load::lbu: {
    return mem.read_byte(addr);
  }
  case load::lhu: {
    return mem.read_half(addr);
  }
  default:
    throw std::runtime_error(""); // illegal
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
  mode.mode = mode_tmp;
  csrf.write(static_cast<uint32_t>(csr::mie), 1);
}

void iss_model::handle_sret() {
  std::bitset<32> sstat{csrf.read(to_int(csr::sstatus))};
  sstat[status::sie] = sstat[status::spie];
  mode.mode = static_cast<privilege>(static_cast<uint8_t>(sstat[status::spp]));
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
