// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

void handle_trap() {
  asm("j base");
}

void base(){
  asm ("mret");
}

int main() {
  int result;
  asm("csrr %0, 0" : "=r"(result));
  void (*ptr)() = &handle_trap;
  asm("add t0, %0, x0" : : "r"(ptr));
  asm("csrrw zero, mtvec, t0");
  asm("csrrw zero, mhartid, t0"); // trigger illegal instruction

  return 0;
}