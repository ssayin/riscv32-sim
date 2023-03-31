// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef COMMON_OFFSET_HPP
#define COMMON_OFFSET_HPP

#include "masks.hpp"
#include "types.hpp"

constexpr auto fillbits(UnsignedIntegral auto bitcount) {
  return (1U << bitcount) - 1U;
}

constexpr auto offset(UnsignedIntegral auto inst, UnsignedIntegral auto low,
                      UnsignedIntegral auto high) {
  return (inst >> low) & fillbits(high - low + 1U);
}

#endif // COMMON_OFFSET_HPP
