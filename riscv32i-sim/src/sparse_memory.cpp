#include "sparse_memory.hpp"
#include <iostream>

void sparse_memory::load_program(uint32_t virt_addr, void *ptr,
                                 uint32_t size_in_bytes) {
  if (virt_addr + size_in_bytes >= rom_size)
    throw std::runtime_error("computer rom is not big enough");
  std::memcpy(rom.get() + virt_addr, ptr, size_in_bytes);
  program_end = virt_addr + size_in_bytes;
}

uint8_t sparse_memory::read_byte(uint32_t off) {
  if (off < program_end)
    return rom[off];
  else
    return page[off & 0xFFFFF000].get()[offset<0u, 11u>(off)];
}

// FIXME: reading at page boundary is A PROBLEM
uint16_t sparse_memory::read_half(uint32_t off) {
  return read_byte(off) | (read_byte(off + 1) << 8);
}

uint32_t sparse_memory::read_word(uint32_t off) {
  return read_half(off) | (read_half(off + 2) << 16);
}

void sparse_memory::write_byte(uint32_t off, uint8_t b) {
  if (off < program_end) {
    std::cout << "writing byte to rom? at: " << std::hex << off << std::endl;
    rom[off] = b;
  }
  uint32_t maskd = off & 0xFFFFF000;
  if (!page.contains(maskd))
    page.emplace(maskd, std::make_unique<uint8_t[]>(4096));
  page[maskd].get()[offset<0u, 11u>(off)] = b;
}

void sparse_memory::write_half(uint32_t off, uint16_t h) {
  write_byte(off, offset<0u, 7u>(h));
  write_byte(off + 1, offset<8u, 15u>(h));
}

void sparse_memory::write_word(uint32_t off, uint32_t w) {
  write_half(off, offset<0u, 15u>(w));
  write_half(off + 2, offset<16u, 31u>(w));
}
