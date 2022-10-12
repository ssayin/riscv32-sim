#include "sparse_memory.hpp"
#include "rv32_isn.hpp"
#include <iostream>

uint32_t sparse_memory::ensure_page_exists(uint32_t addr) {
  uint32_t key = addr & 0xFFFFF000;
  if (!page.contains(key))
    page.emplace(key, std::make_unique<uint8_t[]>(4096));
  return key;
}

void sparse_memory::write_blocks(uint32_t virt_addr, void *ptr,
                                 int64_t size_in_bytes) {
  while (size_in_bytes > 0) {
    uint32_t key      = ensure_page_exists(virt_addr);
    uint32_t off      = offset<0u, 11u>(virt_addr);
    uint32_t page_end = 4096 - off;
    size_in_bytes -= page_end;
    write_block(&page[key].get()[offset<0u, 11u>(virt_addr)], ptr, page_end);
    virt_addr += page_end;
    ptr = (uint8_t *)ptr + page_end;
  }
}

void *sparse_memory::write_block(uint8_t *page_offset, void *ptr,
                                 uint32_t size_in_bytes) {
  std::memcpy(page_offset, ptr, size_in_bytes);
  return (uint8_t *)ptr + size_in_bytes;
}

// TODO: add memory address decoder as a separate class.
uint8_t sparse_memory::read_byte(uint32_t off) {
  return page[off & 0xFFFFF000].get()[offset<0u, 11u>(off)];
}

// FIXME: reading at page boundary is A PROBLEM
// TODO: impl. unaligned read later
uint16_t sparse_memory::read_half(uint32_t off) {
  return read_byte(off) | (read_byte(off + 1) << 8);
}

uint32_t sparse_memory::read_word(uint32_t off) {
  return read_half(off) | (read_half(off + 2) << 16);
}

void sparse_memory::write_byte(uint32_t off, uint8_t b) {
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
