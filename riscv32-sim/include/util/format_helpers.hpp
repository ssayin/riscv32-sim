// SPDX-FileCopyrightText: 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef UTIL_FORMAT_HELPERS_HPP
#define UTIL_FORMAT_HELPERS_HPP

#include <array>
#include <iterator>
#include <ostream>

#include "spdlog/fmt/bundled/core.h"
#include "spdlog/fmt/bundled/format.h"
#include "spdlog/fmt/bundled/os.h"
#include "spdlog/fmt/bundled/ostream.h"

#include "../trap_cause.hpp"
#include "common/hart_state.hpp"

extern "C" {
#include <riscv-disas.h>
}

constexpr std::string_view str_trap_cause(trap_cause cause) {
  switch (cause) {
  case trap_cause::exp_inst_addr_misaligned:
    return "misaligned instruction address";
  case trap_cause::exp_inst_access_fault:
    return "instruction access fault";
  case trap_cause::exp_inst_illegal:
    return "illegal instruction";
  case trap_cause::exp_breakpoint:
    return "breakpoint";
  case trap_cause::exp_load_addr_misaligned:
    return "misaligned load address";
  case trap_cause::exp_load_access_fault:
    return "load access fault";
  case trap_cause::exp_store_addr_misaligned:
    return "misaligned store address";
  case trap_cause::exp_store_access_fault:
    return "store access fault";
  case trap_cause::exp_ecall_from_u_vu_mode:
    return "ecall from user / virtual user mode";
  case trap_cause::exp_ecall_from_hs_mode:
    return "ecall from supervisor mode";
  case trap_cause::exp_ecall_from_vs_mode:
    return "ecall from virtual supervisor mode";
  case trap_cause::exp_ecall_from_m_mode:
    return "ecall from machine mode";
  case trap_cause::exp_inst_page_fault:
    return "instruction page fault";
  case trap_cause::exp_load_page_fault:
    return "load page fault";
  case trap_cause::exp_store_page_fault:
    return "store page fault";
  case trap_cause::exp_inst_guest_page_fault:
    return "instruction guest page fault";
  case trap_cause::exp_load_guest_page_fault:
    return "load guest page fault";
  case trap_cause::exp_inst_virtual:
    return "virtual instruction";
  case trap_cause::exp_store_guest_page_fault:
    return "store guest page fault";
  case trap_cause::int_sw_s:
    return "supervisor software interrupt";
  case trap_cause::int_sw_vs:
    return "virtual supervisor software interrupt";
  case trap_cause::int_sw_m:
    return "machine software interrupt";
  case trap_cause::int_timer_s:
    return "supervisor timer interrupt";
  case trap_cause::int_timer_vs:
    return "virtual supervisor timer interrupt";
  case trap_cause::int_timer_m:
    return "machine timer interrupt";
  case trap_cause::int_external_s:
    return "supervisor external interrupt";
  case trap_cause::int_external_vs:
    return "virtual supervisor external interrupt";
  case trap_cause::int_external_m:
    return "machine external interrupt";
  case trap_cause::int_external_s_guest:
    return "supervisor guest external interrupt";
  default:
    return "unknown";
  }
}

template <>
struct fmt::formatter<trap_cause> : fmt::formatter<std::string_view> {
  auto format(trap_cause tc, fmt::format_context &ctx) const {
    return fmt::formatter<std::string_view>::format(str_trap_cause(tc), ctx);
  }
};

template <> struct fmt::formatter<csr_change> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const csr_change &change, FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "CSR[{:>#5x}]: {:>#10x} => {:>#10x}",
                          change.index, change.prev, change.next);
  }
};

template <> struct fmt::formatter<gpr_change> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const gpr_change &change, FormatContext &ctx) {
    // set #5x to align with CSR
    return fmt::format_to(ctx.out(), "GPR[{:>#5x}]: {:>#10x} => {:>#10x}",
                          change.index, change.prev, change.next);
  }
};

constexpr std::string_view target_str(const target &tgt) {
  switch (tgt) {
  case target::load:
    return "load";
  case target::store:
    return "store";
  case target::alu:
    return "alu";
  case target::branch:
    return "branch";
  case target::csr:
    return "csr";
  case target::mret:
    return "mret";
  case target::illegal:
    return "illegal";
  case target::ebreak:
    return "ebreak";
  case target::ecall:
    return "ecall";
  }

  return "unknown";
}

constexpr std::string_view op_type_str(const op_type &type) {
  if (std::holds_alternative<std::monostate>(type)) {
    return "empty";
  } else if (std::holds_alternative<alu>(type)) {
    const auto &val = std::get<alu>(type);
    switch (val) {
    case alu::_and:
      return "and";
    case alu::_or:
      return "or";
    case alu::_xor:
      return "xor";
    case alu::_add:
      return "add";
    case alu::_sub:
      return "sub";
    case alu::_sll:
      return "sll";
    case alu::_srl:
      return "srl";
    case alu::_sra:
      return "sra";
    case alu::_slt:
      return "slt";
    case alu::_sltu:
      return "sltu";
    case alu::_mul:
      return "mul";
    case alu::_mulh:
      return "mulh";
    case alu::_mulhsu:
      return "mulhsu";
    case alu::_mulhu:
      return "mulhu";
    case alu::_div:
      return "div";
    case alu::_divu:
      return "divu";
    case alu::_rem:
      return "rem";
    case alu::_remu:
      return "remu";
    case alu::_auipc:
      return "auipc";
    case alu::_jal:
      return "jal";
    case alu::_jalr:
      return "jalr";
    }
  } else if (std::holds_alternative<masks::load>(type)) {
    const auto &val = std::get<masks::load>(type);
    switch (val) {
    case masks::load::lb:
      return "lb";
    case masks::load::lh:
      return "lh";
    case masks::load::lw:
      return "lw";
    case masks::load::lbu:
      return "lbu";
    case masks::load::lhu:
      return "lhu";
    }
  } else if (std::holds_alternative<masks::store>(type)) {
    const auto &val = std::get<masks::store>(type);
    switch (val) {
    case masks::store::sb:
      return "sb";
    case masks::store::sh:
      return "sh";
    case masks::store::sw:
      return "sw";
    }
  } else if (std::holds_alternative<masks::branch>(type)) {
    const auto &val = std::get<masks::branch>(type);
    switch (val) {
    case masks::branch::beq:
      return "beq";
    case masks::branch::bne:
      return "bne";
    case masks::branch::blt:
      return "blt";
    case masks::branch::bge:
      return "bge";
    case masks::branch::bltu:
      return "bltu";
    case masks::branch::bgeu:
      return "bgeu";
    }
  } else if (std::holds_alternative<masks::sys>(type)) {
    const auto &val = std::get<masks::sys>(type);
    switch (val) {
    case masks::sys::csrrw:
      return "csrrw";
    case masks::sys::csrrs:
      return "csrrs";
    case masks::sys::other:
      return "other";
    case masks::sys::csrrc:
      return "csrrc";
    case masks::sys::csrrwi:
      return "csrrwi";
    case masks::sys::csrrsi:
      return "csrrsi";
    case masks::sys::csrrci:
      return "csrrci";
    }
  }
  return "unknown";
}

template <> struct fmt::formatter<op> {

  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  }

  /*template <typename FormatContext>
  auto format(const op &value, FormatContext &ctx) {
    return fmt::format_to(
        ctx.out(),
        "imm={}\ntgt={}\nopt={}\nrd={}\nrs1={}\nrs2={}\nhas_imm={}\n"
        "use_pc={}\nis_compressed={}",
        value.imm, static_cast<uint8_t>(value.tgt), op_type_str(value.opt),
        value.rd, value.rs1, value.rs2, value.has_imm, value.use_pc,
        value.is_compressed);
  }
  */

  template <typename FormatContext>
  auto format(const op &value, FormatContext &ctx) {
    return fmt::format_to(ctx.out(),
                          "+--------------+-----------------+\n"
                          "|{0:<14}|{1:<17}|\n"
                          "+--------------+-----------------+\n"
                          "|{2:<14}|{3:<17}|\n"
                          "+--------------+-----------------+\n"
                          "|{4:<14}|{5:<17}|\n"
                          "+--------------+-----------------+\n"
                          "|{6:<14}|{7:<17}|\n"
                          "+--------------+-----------------+\n"
                          "|{8:<14}|{9:<17}|\n"
                          "+--------------+-----------------+\n"
                          "|{10:<14}|{11:<17}|\n"
                          "+--------------+-----------------+\n"
                          "|{12:<14}|{13:<17}|\n"
                          "+--------------+-----------------+\n"
                          "|{14:<14}|{15:<17}|\n"
                          "+--------------+-----------------+\n"
                          "|{16:<14}|{17:<17}|\n"
                          "+--------------+-----------------+\n"
                          "|{18:<14}|{19:<17}|\n"
                          "+--------------+-----------------+\n",
                          "Field", "Value", "imm", value.imm, "tgt",
                          target_str(value.tgt), "opt", op_type_str(value.opt),
                          "rd", value.rd, "rs1", value.rs1, "rs2", value.rs2,
                          "has_imm", value.has_imm, "use_pc", value.use_pc,
                          "is_compressed", value.is_compressed);
  }
};

#endif /* end of include guard: UTIL_FORMAT_HELPERS_HPP */
