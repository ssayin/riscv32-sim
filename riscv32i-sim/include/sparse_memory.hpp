#pragma once

#include <cassert>
#include <cstring>
#include <memory>
#include <unordered_map>

#include "Defs.hpp"

class sparse_memory {
  std::unordered_map<uint32_t, std::unique_ptr<uint8_t[]>> page;

public:
  uint8_t  read_byte(uint32_t off);
  uint16_t read_half(uint32_t off);
  uint32_t read_word(uint32_t off);
  void     write_byte(uint32_t off, uint8_t b);
  void     write_half(uint32_t off, uint16_t h);
  void     write_word(uint32_t off, uint32_t w);
};
