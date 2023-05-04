// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef ISS_MODEL_HPP
#define ISS_MODEL_HPP

#include <array>
#include <atomic>

#include "common/hart_state.hpp"
#include "common/program_counter.hpp"
#include "common/types.hpp"
#include "loader.hpp"
#include "memory/sparse_memory.hpp"
#include "options.hpp"
#include "reg_file.hpp"
#include "trap_cause.hpp"
#include "zicsr/csr_file.hpp"

namespace iss {
class model {

public:
  class unexpected_switch_case_value : public std::runtime_error {
  public:
    unexpected_switch_case_value()
        : std::runtime_error("Unexpected value in switch statement") {}
  };

  model(options &opt, loader l, mem::address_router &mem)
      : allowed_pcs{l.progbit_ranges()}, opts{opt},
        tohost_addr{l.symbol(opt.tohost_sym)}, cur_state{l.entry()}, mem{mem} {}

  void step();
  void commit();

  const hart_state &state() const { return cur_state; }

  uint32_t tohost() { return mem.read32(tohost_addr); }
  bool     done() const { return is_done; }

  void set_pending(trap_cause cause);

private:
  std::vector<std::tuple<uint64_t, uint64_t>> allowed_pcs;
  options                                    &opts;

  void write(csr_file &file, uint16_t index, uint32_t val) {
    file.write(index, val, cur_state.csr_staged);
  }

  void write(reg_file &file, uint8_t index, uint32_t val) {
    file.write(index, val, cur_state.gpr_staged);
  }

  void       trap(trap_cause cause);
  uint32_t   load();
  void       store();
  void       csr();
  void       exec();
  void       fetch();
  void       handle_alu();
  void       handle_load();
  void       handle_branch();
  trap_cause ecall_cause() const;
  void       handle_mret();
  void       handle_sys_exit();
  void       save_pc(const trap_cause &cause);

  const uint32_t tohost_addr;

  hart_state cur_state;

  mem::address_router &mem;
  privileged_mode      mode{privilege::machine};
  reg_file             regf;
  csr_file             csrf;
  bool                 is_done = false;
  void                 interrupt_pending();

  static bool is_address_initialized(
      const std::vector<std::tuple<uint64_t, uint64_t>> &ranges,
      uint64_t                                           address);

  bool is_valid_pc(uint64_t addr) {
    return is_address_initialized(this->allowed_pcs, addr);
  }
};

} // namespace iss

#endif
