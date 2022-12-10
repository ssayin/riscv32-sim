#include <stdint.h>

uint32_t fib(uint8_t v) {
  if(v == 0 || v == 1) return 1;
  else return fib(v - 1) + fib(v - 2);
}

int main() {
  return fib(6); // 13
}