#ifndef RISCV32_SIM_TRAP_CAUSE_HPP
#define RISCV32_SIM_TRAP_CAUSE_HPP

#include <cstdint>

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

constexpr bool is_interrupt(trap_cause cause) {
  switch (cause) {
  case trap_cause::exp_inst_addr_misaligned:
  case trap_cause::exp_inst_access_fault:
  case trap_cause::exp_inst_illegal:
  case trap_cause::exp_breakpoint:
  case trap_cause::exp_load_addr_misaligned:
  case trap_cause::exp_load_access_fault:
  case trap_cause::exp_store_addr_misaligned:
  case trap_cause::exp_store_access_fault:
  case trap_cause::exp_ecall_from_u_vu_mode:
  case trap_cause::exp_ecall_from_hs_mode:
  case trap_cause::exp_ecall_from_vs_mode:
  case trap_cause::exp_ecall_from_m_mode:
  case trap_cause::exp_inst_page_fault:
  case trap_cause::exp_load_page_fault:
  case trap_cause::exp_store_page_fault:
  case trap_cause::exp_inst_guest_page_fault:
  case trap_cause::exp_load_guest_page_fault:
  case trap_cause::exp_inst_virtual:
  case trap_cause::exp_store_guest_page_fault:
  default:
    return false;
  case trap_cause::int_sw_s:
  case trap_cause::int_sw_vs:
  case trap_cause::int_sw_m:
  case trap_cause::int_timer_s:
  case trap_cause::int_timer_vs:
  case trap_cause::int_timer_m:
  case trap_cause::int_external_s:
  case trap_cause::int_external_vs:
  case trap_cause::int_external_m:
  case trap_cause::int_external_s_guest:
    return true;
  }
}

#endif // RISCV32_SIM_TRAP_CAUSE_HPP
