#include <stdint.h>

#include "start.h"

int main() {
  int result;
  asm("csrr %0, 0" : "=r"(result));

  return 0;
}
