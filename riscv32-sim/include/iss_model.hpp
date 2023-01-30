#ifndef ISS_MODEL_HPP
#define ISS_MODEL_HPP

#include "decoder/decoder.hpp"
#include "loader.hpp"
#include "memory/sparse_memory.hpp"
#include "mti_source.hpp"
#include "nlohmann/detail/macro_scope.hpp"
#include "nlohmann/json_fwd.hpp"
#include "program_counter.hpp"
#include "reg_file.hpp"
#include "zicsr/csr.hpp"
#include "zicsr/csr_file.hpp"
#include "zicsr/trap_cause.hpp"
#include <algorithm>
#include <fmt/os.h>
#include <fmt/ostream.h>
#include <functional>
#include <nlohmann/json.hpp>
#include <numeric>
#include <thread>

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

  uint8_t read8(uint32_t off) const;
  void    write8(uint32_t off, uint8_t b);
};

struct state {
  uint32_t instr;
  op       dec;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(state, instr, dec)

class iss_model {
  using json = nlohmann::json;

public:
  iss_model(options &opt, loader l, address_router &mem)
      : opts{opt},
        tohost_addr{l.symbol(opt.tohost_sym)}, mem{mem}, pc{l.entry()} {}

  void step();
  void trace(fmt::ostream &out);

  uint32_t tohost() { return mem.read32(tohost_addr); }
  bool     done() const { return is_done; }

  void set_pending(trap_cause cause);

  json j;

private:
  options             &opts;
  static constexpr int instr_alignment = 4;

  void trap(trap_cause cause);

  uint32_t load();
  void     store();
  void     csr();
  void     exec();
  op       next_op();
  void     handle_alu();
  void     handle_load();
  void     handle_branch();

  trap_cause ecall_cause() const;

  void handle_mret();
  void handle_sys_exit();
  void save_pc(const trap_cause &cause);

  const uint32_t tohost_addr;

  op       dec;
  uint32_t instr;

  address_router &mem;
  program_counter pc;
  privileged_mode mode{privilege::machine};
  reg_file        regf;
  csr_file        csrf;
  bool            is_done = false;
};

#endif
