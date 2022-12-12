#include "sparse_memory.hpp"
#include "rv32_isn.hpp"
#include <fmt/format.h>

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
    uint32_t off      = offset<0u, 11u>(virt_addr);
    uint32_t page_end = page_size - off;
    ptr               = write_block(&page[key].get()[off], ptr, page_end);
    size_in_bytes -= page_end;
    virt_addr += page_end;
  }
}

uint8_t sparse_memory::read_byte(uint32_t off) {
  if (!page.contains(off & mask))
    throw std::runtime_error(
        fmt::format("{} {} tried to read uninitialized memory {:x}", __FILE__,
                    __LINE__, off));
  return page[off & mask].get()[offset<0u, 11u>(off)];
}

uint16_t sparse_memory::read_half(uint32_t off) {
  return read_byte(off) | (read_byte(off + 1) << 8);
}

uint32_t sparse_memory::read_word(uint32_t off) {
  return read_half(off) | (read_half(off + 2) << 16);
}

void sparse_memory::write_byte(uint32_t off, uint8_t b) {
  uint32_t key                          = ensure_page_exists(off);
  page[key].get()[offset<0u, 11u>(off)] = b;
}

void sparse_memory::write_half(uint32_t off, uint16_t h) {
  write_byte(off, offset<0u, 7u>(h));
  write_byte(off + 1, offset<8u, 15u>(h));
}

void sparse_memory::write_word(uint32_t off, uint32_t w) {
  write_half(off, offset<0u, 15u>(w));
  write_half(off + 2, offset<16u, 31u>(w));
}
