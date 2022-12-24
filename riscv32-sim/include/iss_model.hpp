#ifndef ISS_MODEL_HPP
#define ISS_MODEL_HPP

#include "decoder/decoder.hpp"
#include "loader.hpp"
#include "memory/sparse_memory.hpp"
#include "program_counter.hpp"
#include "reg_file.hpp"
#include "zicsr/csr_file.hpp"
#include "zicsr/privilege_level.hpp"

class iss_model {
public:
  iss_model(loader l, sparse_memory &mem);

  void step();

  bool done() const { return is_done; }

  uint32_t tohost() { return mem.read_word(tohost_addr); }

  const uint32_t tohost_addr;

  privilege_level mode = privilege_level::machine;

private:
  void     csr(op &dec);
  uint32_t load(op &dec);
  void     store(op &dec);
  uint32_t alu(op &dec);

  void handle_mret();
  void handle_sret();

  void handle_sys_exit();

  sparse_memory  &mem;
  program_counter pc;
  reg_file        regf;
  csr_file        csrf;

  bool is_done = false;
};

#endif
