#include <stdint.h>

extern void _start() __attribute__ ((noreturn));
extern void _exit(int exit_code) __attribute__ ((noreturn,noinline));

extern int  main();

volatile uint32_t tohost;

void _exit(int ret) { tohost = ret; }

void _start() {
  int ret = main();
  _exit(ret);
}
