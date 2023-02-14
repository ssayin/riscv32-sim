#ifndef ISS_MODEL_HPP
#define ISS_MODEL_HPP

#include "common/program_counter.hpp"
#include "common/trap_cause.hpp"
#include "common/types.hpp"
#include "loader.hpp"
#include "memory/sparse_memory.hpp"
#include "options.hpp"
#include "reg_file.hpp"
#include "zicsr/csr_file.hpp"

#include <array>
#include <atomic>
#include <fmt/os.h>

#include "common/op.hpp"

/*
 * Acts as a crossbar switch CPU <=> Mem RW
 */
class address_router : public sparse_memory_accessor {
private:
  uint32_t                            mtime_addr;
  uint32_t                            mtimecmp_addr;
  std::array<std::atomic<uint8_t>, 8> mtimecmp;

public:
  std::array<std::atomic<uint8_t>, 8> mtime;

  address_router(sparse_memory &mem, uint32_t mtime_addr,
                 uint32_t mtimecmp_addr);

  using sparse_memory_accessor::read16;
  using sparse_memory_accessor::read32;
  using sparse_memory_accessor::read64;
  using sparse_memory_accessor::write16;
  using sparse_memory_accessor::write32;
  using sparse_memory_accessor::write64;

  [[nodiscard]] uint8_t read8(uint32_t off) const;
  void                  write8(uint32_t off, uint8_t b);
};

class iss_model {

public:
  iss_model(options &opt, loader l, address_router &mem)
      : opts{opt},
        tohost_addr{l.symbol(opt.tohost_sym)}, mem{mem}, pc{l.entry()} {}

  void step();
  void trace_disasm(fmt::ostream &out);

  template <class Container> void trace(Container &cont) {
    cont.emplace_back(cur_state);
  }

  uint32_t           tohost() { return mem.read32(tohost_addr); }
  [[nodiscard]] bool done() const { return is_done; }

  void set_pending(trap_cause cause);

private:
  options &opts;

  void trap(trap_cause cause);

  uint32_t load();
  void     store();
  void     csr();
  void     exec();
  void     fetch();
  void     handle_alu();
  void     handle_load();
  void     handle_branch();

  [[nodiscard]] trap_cause ecall_cause() const;

  void handle_mret();
  void handle_sys_exit();
  void save_pc(const trap_cause &cause);

  const uint32_t tohost_addr;

  hart_state cur_state;

  address_router &mem;
  program_counter pc;
  privileged_mode mode{privilege::machine};
  reg_file        regf;
  csr_file        csrf;
  bool            is_done = false;
  void            interrupt_pending();
};

#endif
