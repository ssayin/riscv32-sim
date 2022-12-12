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
  int stat;

  while (!(stat = model.done())) {
    model.step();
  }

  fmt::print("Exited with {}\n", stat + 100000);

  return 0;
}
