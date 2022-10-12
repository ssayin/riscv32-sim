#include <stdint.h>

volatile uint32_t tohost;

int main() {
  uint32_t a = 1;
  uint32_t b = 2;
  uint32_t c = (a + b) >> 3;

  return 0;
}

void _exit(int ret) { tohost = 0xBADDAD; }

void _start() {
  int ret = main();
  _exit(ret);
}
