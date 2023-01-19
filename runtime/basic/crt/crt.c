#include <stdint.h>

extern void _entry() __attribute__((naked, noreturn, noinline));
extern void _start() __attribute__((noreturn));
extern void _exit(int exit_code) __attribute__((naked, noreturn, noinline));

void _entry() {
  __asm__ volatile(".option push;"
                   ".option norelax;"
                   "la gp, __global_pointer$;"
                   ".option pop;"
                   "jal zero, _start;");
}

extern int main();

volatile uint32_t tohost;

void _exit(int ret) { tohost = ret; }

void _start() {
  int ret = main();
  _exit(ret);
}
