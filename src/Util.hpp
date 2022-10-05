#pragma once

#include <bitset>
#include <numeric>
#include <type_traits>

template <class T>
concept Unsigned = std::is_unsigned<T>::value;

template <auto x = 1u> consteval auto fillbits(Unsigned auto bitcount) {
  static_assert(x == 0u || x == 1u);
  if (bitcount == 0u)
    static_assert("fillbits: number of bits cannot be 0");
  auto sum = 1u;
  while (--bitcount) {
    sum = (sum << 1u) | x;
  }
  return sum;
}

template <Unsigned auto l, Unsigned auto h>
using diff = std::integral_constant<decltype(h - l + 1u), h - l + 1u>;

template <Unsigned auto l, Unsigned auto h>
decltype(auto) offset(Unsigned auto inst) {
  return (inst >> l) & fillbits(diff<l, h>::value);
}

template <Unsigned auto l, Unsigned auto h>
using BitSet = std::bitset<diff<l, h>::value>;
