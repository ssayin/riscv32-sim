// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include <stdint.h>

int main() {
  uint32_t a = 1;
  uint32_t b = 2;
  uint32_t c = (a + b) << 3;

  return c; // 24
}