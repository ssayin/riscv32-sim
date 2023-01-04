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

class iss_model {
public:
  iss_model(loader l, sparse_memory &mem);
  void     step();
  uint32_t tohost() { return mem.read_word(tohost_addr); }
  bool     done() const { return is_done; }

  uint16_t priv_base() const {
    return static_cast<uint16_t>(to_int(mode) << 8);
  }

  uint16_t priv_csr(enum csr c) const { return priv_base() | to_int(c); }

private:
  uint32_t alu(op &dec);
  uint32_t load(op &dec);
  void     store(op &dec);
  void     csr(op &dec);

  void handle_mret();
  void handle_sret();
  void handle_sync_exception(sync_exception &ex);
  void handle_sys_exit();

  const uint32_t tohost_addr;

  sparse_memory  &mem;
  program_counter pc;
  privilege       mode;
  reg_file        regf;
  csr_file        csrf;
  bool            is_done;
};

#endif
