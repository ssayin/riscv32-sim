#include <stdint.h>

void expc() __attribute__((interrupt("machine")));
void timer() __attribute__((interrupt("machine")));
void base() __attribute__((naked));

volatile void base() {
  __asm__ volatile(".org base + 0;"
                   "jal zero, expc;"
                   ".org base + 7*4;"
                   "jal zero, timer;");
}

volatile uint64_t mtime;
volatile uint64_t mtimecmp;

uint32_t acc;

void expc() {}

void timer() {
  acc += (mtime - mtimecmp);
  mtimecmp = mtime;
}

int main() {
  acc           = 0;
  mtime         = 0;
  mtimecmp      = 0;
  void (*ptr)() = &base;
  asm("add t0, %0, x0" : : "r"(ptr));
  asm("csrrw zero, mtvec, t0");
  asm("csrrw zero, mie, t0");

  int i = 0;
  while (++i < 1000)
    ;

  return acc;
}