#include "csr_file.hpp"
#include "common/trap_cause.hpp"
#include "offset.hpp"
#include "rv32_isn.hpp"
#include "sync_exception.hpp"

static uint8_t priv(uint32_t addr);

uint32_t csr_file::read(uint32_t addr) {
  if (priv(addr) > to_int(mode))
    throw sync_exception(trap_cause::exp_inst_illegal);
  return csrs[addr];
}

void csr_file::write(uint32_t addr, uint32_t v) {
  auto is_readonly = [](uint32_t addr) {
    return offset<10u, 11u>(addr) == 0b11;
  };
  if (is_readonly(addr) || (priv(addr) > to_int(mode)))
    throw sync_exception(trap_cause::exp_inst_illegal);
  csrs[addr] = v;
}

static uint8_t priv(uint32_t addr) { return offset<8u, 9u>(addr); };