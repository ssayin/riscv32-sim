#include "csr_handler.hpp"
#include "rv32_isn.hpp"

void csr_handler::write(uint32_t addr, uint32_t v) {
  uint8_t priv = offset<8u, 11u>(addr);
  if (is_unimplemented_csr(addr) || ((priv & 0xC) == 0xC) ||
      (priv & 0x3) > mode) {
    // TODO: Impl. trap
    return;
  }
  csrs[addr] = v;
}
uint32_t csr_handler::read(uint32_t addr) {
  if (offset<8u, 9u>(addr) > mode || is_unimplemented_csr(addr)) {
    // TODO: Impl. trap
    return 0x0;
  }

  return csrs[addr];
}
bool csr_handler::is_unimplemented_csr(uint32_t addr) {
  return addr >= 0x7A0 && addr <= 0x7BF;
}

csr_handler::csr_handler() { write(csr::misa, "IMS"_MISA | RV32); }

void csr_handler::write(csr addr, uint32_t v) {
  write(static_cast<uint32_t>(addr), v);
}

uint32_t csr_handler::read(csr addr) { read(static_cast<uint32_t>(addr)); }
