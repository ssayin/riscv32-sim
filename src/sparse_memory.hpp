#pragma once

#include "Util.hpp"
#include <memory>
#include <unordered_map>

class Memory {
  std::unordered_map<uint32_t, std::unique_ptr<uint8_t[]>> page;

public:
  uint8_t read_byte(uint32_t off) {
    return page[off & 0xFFFFF000].get()[offset<0u, 11u>(off)];
  }

  // FIXME: reading at page boundary is A PROBLEM
  uint16_t read_half(uint32_t off) {
    return read_byte(off) | (read_byte(off + 1) << 8);
  }

  uint32_t read_word(uint32_t off) {
    return read_half(off) | (read_half(off + 2) << 16);
  }

  void write_byte(uint32_t off, uint8_t b) {
    uint32_t maskd = off & 0xFFFFF000;
    if (!page.contains(maskd))
      page.emplace(maskd, std::make_unique<uint8_t[]>(4096));
    page[maskd].get()[offset<0u, 11u>(off)] = b;
  }

  void write_half(uint32_t off, uint16_t h) {
    write_byte(off, offset<0u, 7u>(h));
    write_byte(off + 1, offset<8u, 15u>(h));
  }

  void write_word(uint32_t off, uint32_t w) {
    write_half(off, offset<0u, 15u>(w));
    write_half(off + 2, offset<16u, 31u>(w));
  }
};
