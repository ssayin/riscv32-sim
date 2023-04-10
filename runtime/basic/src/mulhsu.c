// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include "serdar_core.h"

int main() {
  register uint32_t a asm("a1") = 0xEFFFFFFF;
  register int32_t  b asm("a2") = -1;
  register int32_t  c asm("a3") = 0;
  asm("mulhsu a3, a2, a1");
  return c;
}
