#pragma once

#include <cassert>
#include <cstring>
#include <memory>
#include <unordered_map>

class sparse_memory {
  std::unordered_map<uint32_t, std::unique_ptr<uint8_t[]>> page;

  void    *write_block(uint8_t *page_offset, void *ptr, uint32_t size_in_bytes);
  uint32_t ensure_page_exists(uint32_t addr);

public:
  void     write_blocks(uint32_t virt_addr, void *ptr, int64_t size_in_bytes);
  uint8_t  read_byte(uint32_t off);
  uint16_t read_half(uint32_t off);
  uint32_t read_word(uint32_t off);
  void     write_byte(uint32_t off, uint8_t b);
  void     write_half(uint32_t off, uint16_t h);
  void     write_word(uint32_t off, uint32_t w);
};
