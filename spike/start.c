#include <stdint.h>

int main();

volatile uint32_t tohost;

void _exit(int ret) { tohost = 0xBADDAD; }

void _start() {
  int ret = main();
  _exit(ret);
}
