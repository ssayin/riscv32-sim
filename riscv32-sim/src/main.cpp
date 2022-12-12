#include "iss_model.hpp"
#include "loader.hpp"
#include "sparse_memory.hpp"
#include <cstdio>

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: riscv32-sim <file_name>\n");
    return 1;
  }

  sparse_memory mem;
  iss_model     model{loader(argv[1], mem), mem};

  while (!model.done()) {
    model.step();
  }

  fmt::print("Exited with {}\n", model.tohost());

  return 0;
}
