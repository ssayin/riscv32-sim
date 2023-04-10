// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

/*
 * ./build.sh; ./build/riscv32-sim/riscv32-sim  --timer --mtime 32768 --mtimecmp
 * 33024 --interval 10 ./build/runtime/basic/timer.elf
 */

#include <stdint.h>

// TODO: map to read-only region
#define MTIME 0x8000

#define MTIMECMP 0x8100

/*
 * 1000 / interval
 * interval is set by CLI oprions in sim
 */
#define CLOCK_FREQ 100

void expc() __attribute__((interrupt("machine")));
void timer() __attribute__((interrupt("machine")));
void base() __attribute__((naked));

void base() {
  __asm__ volatile(".org base + 0;"
                   "jal zero, expc;"
                   ".org base + 7*4;"
                   "jal zero, timer;");
}

void expc() { return; }

// https://github.com/five-embeddev/riscv-scratchpad/blob/master/baremetal-startup-c/src/timer.c
static uint64_t mtime() {
  volatile uint32_t *mtimel = (volatile uint32_t *)(MTIME);
  volatile uint32_t *mtimeh = (volatile uint32_t *)(MTIME + 4);
  uint32_t           mtimeh_val;
  uint32_t           mtimel_val;
  do {
    // There is a small risk the mtimeh will tick over after reading mtimel
    mtimeh_val = *mtimeh;
    mtimel_val = *mtimel;
    // Poll mtimeh to ensure it's consistent after reading mtimel
    // The frequency of mtimeh ticking over is low
  } while (mtimeh_val != *mtimeh);
  return (uint64_t)((((uint64_t)mtimeh_val) << 32) | mtimel_val);
}

static uint32_t timer_enabled = 0;

void timer() {
  uint64_t new_mtimecmp = mtime() + CLOCK_FREQ * 10; // interrupt every 100ms
  volatile uint32_t *mtimecmpl = (volatile uint32_t *)(MTIMECMP);
  volatile uint32_t *mtimecmph = (volatile uint32_t *)(MTIMECMP + 4);
  // AS we are doing 32 bit writes, an intermediate mtimecmp value may cause
  // spurious interrupts. Prevent that by first setting the dummy MSB to an
  // unacheivable value
  *mtimecmph = 0xFFFFFFFF; // cppcheck-suppress redundantAssignment
  // set the LSB
  *mtimecmpl = (uint32_t)(new_mtimecmp & 0x0FFFFFFFFUL);
  // Set the correct MSB
  *mtimecmph =
      (uint32_t)(new_mtimecmp >> 32); // cppcheck-suppress redundantAssignment
  timer_enabled = 1;
}

// void busy1337(uint16_t) __attribute__((noinline,
// optimize("no-unroll-loops")));

void busy1337(uint16_t) __attribute__((optimize("O0")));

void busy1337(uint16_t x) {
  int i = 0;

  // #pragma GCC unroll 1
  while (++i < x)
    ;

  // #pragma GCC unroll 1
  while (--i > 0)
    ;
}

int main() {
  void (*ptr)()        = &base;
  const uint32_t value = 0x80;
  __asm__ volatile("csrrw zero, mtvec, %0;" : : "r"(ptr));
  __asm__ volatile("csrrw zero, mie, %0;" : : "r"(value));

  __asm__ volatile("csrrsi zero, mstatus, 0x8;");

  busy1337(100);

  busy1337(100);

  if (timer_enabled)
    return mtime();
  else
    return 0;
}
