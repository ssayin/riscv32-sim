#ifndef ZICSR_TRAP_CAUSE_HPP
#define ZICSR_TRAP_CAUSE_HPP

#include <cstdint>
#include <string_view>

enum class trap_cause : uint32_t {
  exp_inst_addr_misaligned   = 0x0,
  exp_inst_access_fault      = 0x1,
  exp_inst_illegal           = 0x2,
  exp_breakpoint             = 0x3,
  exp_load_addr_misaligned   = 0x4,
  exp_load_access_fault      = 0x5,
  exp_store_addr_misaligned  = 0x6,
  exp_store_access_fault     = 0x7,
  exp_ecall_from_u_vu_mode   = 0x8,
  exp_ecall_from_hs_mode     = 0x9,
  exp_ecall_from_vs_mode     = 0x10,
  exp_ecall_from_m_mode      = 0x11,
  exp_inst_page_fault        = 0x12,
  exp_load_page_fault        = 0x13,
  exp_store_page_fault       = 0x15,
  exp_inst_guest_page_fault  = 0x20,
  exp_load_guest_page_fault  = 0x21,
  exp_inst_virtual           = 0x22,
  exp_store_guest_page_fault = 0x23,
  int_sw_s                   = 0x80000001,
  int_sw_vs                  = 0x80000002,
  int_sw_m                   = 0x80000003,
  int_timer_s                = 0x80000005,
  int_timer_vs               = 0x80000006,
  int_timer_m                = 0x80000007,
  int_external_s             = 0x80000009,
  int_external_vs            = 0x80000010,
  int_external_m             = 0x80000011,
  int_external_s_guest       = 0x80000012
};

// switch-case is a better hint for optimization?
static constexpr std::string_view str_trap_cause(trap_cause cause) {
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

#endif // ZICSR_TRAP_CAUSE_HPP
