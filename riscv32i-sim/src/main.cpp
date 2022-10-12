#include "iss_model.hpp"
#include "loader.hpp"
#include <cstdio>

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: riscv32i-sim <file_name>\n");
    return 1;
  }

  iss_model model;
  loader::load(argv[1], model);

  while (!model.done()) {
    model.step();
  }

  return 0;
}
