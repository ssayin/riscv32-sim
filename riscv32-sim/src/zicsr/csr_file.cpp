#include "common/offset.hpp"
#include "zicsr/csr_file.hpp"
#include "zicsr/sync_exception.hpp"
#include "zicsr/trap_cause.hpp"
#include <fmt/printf.h>

uint8_t priv(uint32_t addr) { return offset(addr, 8U, 9U); };

uint32_t csr_file::read(uint32_t addr) {
  if (priv(addr) > to_int(mode))
    throw sync_exception(trap_cause::exp_inst_illegal);
  return csrs.at(addr);
}

void csr_file::write(uint32_t addr, uint32_t v) {
  auto is_readonly = [](uint32_t addr) {
    return offset(addr, 10U, 11U) == 0b11;
  };
  if (is_readonly(addr) || (priv(addr) > to_int(mode)))
    throw sync_exception(trap_cause::exp_inst_illegal);
  csrs.at(addr) = v;
}
