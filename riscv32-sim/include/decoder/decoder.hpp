#ifndef DECODER_DECODER_HPP
#define DECODER_DECODER_HPP

#include "common/types.hpp"

constexpr inline op make_nop(bool compressed = false) {
  return op{0, alu::_add, target::alu, 0, 0, 0, true, false, compressed};
}

constexpr inline op make_illegal(bool compressed = false) {
  return op{0, {}, target::illegal, 0, 0, 0, false, false, compressed};
}

op decode(uint32_t word);
op decode16(uint16_t word);

#endif // DECODER_DECODER_HPP
