#include "memory/sparse_memory.hpp"
#include "instr/rv32_isn.hpp"
#include <optional>

static void *write_block(uint8_t *page_offset, void *ptr,
                         uint32_t size_in_bytes) {
  std::memcpy(page_offset, ptr, size_in_bytes);
  return (uint8_t *)ptr + size_in_bytes;
}

uint32_t sparse_memory::ensure_page_exists(uint32_t addr) {
  uint32_t key = addr & mask;
  if (!page.contains(key))
    page.emplace(key, std::make_unique<uint8_t[]>(page_size));
  return key;
}

void sparse_memory::load(uint32_t virt_addr, void *ptr, int64_t size_in_bytes) {
  while (size_in_bytes > 0) {
    uint32_t key      = ensure_page_exists(virt_addr);
    uint32_t off      = offset(virt_addr, 0U, 11U);
    uint32_t page_end = page_size - off;
    ptr               = write_block(&page[key].get()[off], ptr, page_end);
    size_in_bytes -= page_end;
    virt_addr += page_end;
  }
}

uint8_t sparse_memory::read_byte(uint32_t off) {
  if (!page.contains(off & mask)) {
    throw std::runtime_error("");
  }
  return page[off & mask].get()[offset(off, 0U, 11U)];
}

uint16_t sparse_memory::read_half(uint32_t off) {
  return read_byte(off) | (read_byte(off + 1) << 8);
}

uint32_t sparse_memory::read_word(uint32_t off) {
  return read_half(off) | (read_half(off + 2) << 16);
}

void sparse_memory::write_byte(uint32_t off, uint8_t b) {
  uint32_t key                          = ensure_page_exists(off);
  page[key].get()[offset(off, 0U, 11U)] = b;
}

void sparse_memory::write_half(uint32_t off, uint16_t h) {
  write_byte(off, offset(h, 0U, 7U));
  write_byte(off + 1, offset(h, 8U, 15U));
}

void sparse_memory::write_word(uint32_t off, uint32_t w) {
  write_half(off, offset(w, 0U, 15U));
  write_half(off + 2, offset(w, 16U, 31U));
}
