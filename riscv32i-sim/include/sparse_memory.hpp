#pragma once

#include <cassert>
#include <cstring>
#include <memory>
#include <unordered_map>

#include "Defs.hpp"

constexpr static uint32_t rom_size = 0x20000;

class sparse_memory {
  std::unordered_map<uint32_t, std::unique_ptr<uint8_t[]>> page;
  std::unique_ptr<uint8_t[]>                               rom;

  uint32_t program_end = 0;

public:
  sparse_memory() { rom = std::make_unique<uint8_t[]>(rom_size); }
  void     load_program(uint32_t virt_addr, void *ptr, uint32_t size_in_bytes);
  uint8_t  read_byte(uint32_t off);
  uint16_t read_half(uint32_t off);
  uint32_t read_word(uint32_t off);
  void     write_byte(uint32_t off, uint8_t b);
  void     write_half(uint32_t off, uint16_t h);
  void     write_word(uint32_t off, uint32_t w);
};
