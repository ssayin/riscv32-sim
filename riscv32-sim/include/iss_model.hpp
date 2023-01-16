#ifndef ISS_MODEL_HPP
#define ISS_MODEL_HPP

#include "decoder/decoder.hpp"
#include "loader.hpp"
#include "memory/sparse_memory.hpp"
#include "program_counter.hpp"
#include "reg_file.hpp"
#include "zicsr/csr.hpp"
#include "zicsr/csr_file.hpp"
#include "zicsr/privilege.hpp"
#include "zicsr/trap_cause.hpp"
#include <fmt/os.h>
#include <fmt/ostream.h>

class iss_model {
public:
  struct opts {
    std::string tohost_sym = "tohost";
    bool        trace      = false;
  };

  iss_model(opts &opt, loader l, sparse_memory &mem)
      : tohost_addr{l.symbol(opt.tohost_sym)}, opt{opt}, mem{mem},
        pc{l.entry()} {}

  void step();
  void trace(fmt::ostream &out);

  uint32_t tohost() { return mem.read_word(tohost_addr); }
  bool     done() const { return is_done; }

private:
  uint32_t load(op &dec);
  void     store(op &dec);
  void     csr(op &dec);
  void     exec(op &dec);
  op       next_op();

  trap_cause handle_ecall() const;
  void       handle_alu(op &dec);
  void       handle_load(op &dec);
  void       handle_branch(const op &dec);

  void handle_mret();
  void handle_sret();
  void handle_sys_exit();
  void save_pc(const trap_cause &cause);

  void handle(trap_cause cause);

  const uint32_t tohost_addr;
  const opts    &opt;

  static constexpr int instr_alignment = 4;

  sparse_memory  &mem;
  program_counter pc;
  privileged_mode mode{privilege::machine};
  reg_file        regf;
  csr_file        csrf;
  bool            is_done = false;
};

#endif
