#include "zicsr/csr_file.hpp"
#include "common/offset.hpp"
#include "instr/rv32_isn.hpp"
#include "zicsr/sync_exception.hpp"
#include "zicsr/trap_cause.hpp"

#include "fmt/printf.h"

static uint8_t priv(uint32_t addr) { return offset<8U, 9U>(addr); };

uint32_t csr_file::read(uint32_t addr) {
  if (priv(addr) > to_int(mode))
    throw sync_exception(trap_cause::exp_inst_illegal);
  return csrs[addr];
}

void csr_file::write(uint32_t addr, uint32_t v) {
  auto is_readonly = [](uint32_t addr) {
    return offset<10U, 11U>(addr) == 0b11;
  };
  if (is_readonly(addr) || (priv(addr) > to_int(mode)))
    throw sync_exception(trap_cause::exp_inst_illegal);

  // fmt::print("csr[{:#x}] <= {:#x}\t", addr, v);
  csrs[addr] = v;
}
