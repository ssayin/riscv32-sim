#ifndef MEMORY_SPARSE_MEMORY_HPP
#define MEMORY_SPARSE_MEMORY_HPP

#include <cassert>
#include <cstring>
#include <memory>
#include <unordered_map>

class sparse_memory {
  std::unordered_map<uint32_t, std::unique_ptr<uint8_t[]>> page;
  uint32_t ensure_page_exists(uint32_t addr);

public:
  constexpr static uint32_t page_size = 4096;
  constexpr static uint32_t mask      = 0xFFFFF000;

  void     load(uint32_t virt_addr, void *ptr, int64_t size_in_bytes);
  uint8_t  read_byte(uint32_t off);
  uint16_t read_half(uint32_t off);
  uint32_t read_word(uint32_t off);
  void     write_byte(uint32_t off, uint8_t b);
  void     write_half(uint32_t off, uint16_t h);
  void     write_word(uint32_t off, uint32_t w);
};

#endif // MEMORY_SPARSE_MEMORY_HPP
