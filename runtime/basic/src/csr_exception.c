// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

extern void handle_trap() __attribute__((interrupt("machine")));

// empty
void handle_trap() {}

int main() {
  int result;
  asm("csrr %0, 0" : "=r"(result));
  asm("add t0, %0, x0" : : "r"(handle_trap));
  asm("csrrw zero, mtvec, t0");
  asm("csrrw zero, mhartid, t0"); // trigger illegal instruction

  return 0;
}
