// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include "serdar_core.h"

int main() {
  uint32_t a = 0xFEED;
  uint32_t b = 0xBEEF;
  uint32_t c;
  asm("divu %0, %1, %2" : "=r"(c) : "r"(a), "r"(b));
  return c;
}
