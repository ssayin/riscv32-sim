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
#include "zicsr/sync_exception.hpp"
#include <fmt/os.h>
#include <fmt/ostream.h>

struct iss_model_opts {
  std::string tohost_sym = "tohost";
  bool        trace      = false;
};

class iss_model {
public:
  iss_model(iss_model_opts &opts, loader l, sparse_memory &mem)
      : tohost_addr{l.symbol(opts.tohost_sym)}, opts{opts}, mem{mem},
        pc{l.entry()}, csrf(mode) {}

  void     step();
  void     trace(fmt::ostream &out);

  uint32_t tohost() { return mem.read_word(tohost_addr); }
  bool     done() const { return is_done; }

private:
  uint32_t load(op &dec);
  void     store(op &dec);
  void     csr(op &dec);
  void     exec(op &dec);
  op       next_op();

  void     dispatch_ecall() const;
  void     handle_alu(op &dec);
  void     handle_load(op &dec);
  void     handle_branch(const op &dec);

  void handle_mret();
  void handle_sret();
  void handle(sync_exception &ex);
  void handle_sys_exit();

  const uint32_t tohost_addr;

  const iss_model_opts &opts;

  sparse_memory  &mem;
  program_counter pc;
  privilege       mode{privilege::machine};
  reg_file        regf;
  csr_file        csrf;
  bool            is_done = false;
  void            save_pc(const trap_cause &cause);
};

#endif
