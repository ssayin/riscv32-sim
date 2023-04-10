// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include "serdar_core.h"

extern void _entry() __attribute__((naked, section(".text.init")));
extern void _start() __attribute__((noreturn));
extern void _exit(int) __attribute__((noreturn, noinline));

void _entry() {
  __asm__ volatile(".option push;"
                   ".option norelax;"
                   "la gp, __global_pointer$;"
                   ".option pop;"
                   "la sp, _sp;"
                   "jal zero, _start;");
}

extern int main();

volatile uint32_t tohost;

void _exit(int ret) { tohost = ret; }

void _start() {
  int ret = main();
  _exit(ret);
}
