#include "iss_model.hpp"
#include "loader.hpp"
#include "memory/sparse_memory.hpp"

int main(int argc, char **argv) {
  if (argc != 2) {
    fmt::print("Usage: riscv32-sim <file_name>\n");
    return 1;
  }

  sparse_memory mem;
  iss_model     model{loader(argv[1], mem), mem};

  while (!model.done()) {
    model.step();
  }

  fmt::print("{} Exited with 0x{:X} ({})\n", argv[0], model.tohost(),
             static_cast<int32_t>(model.tohost()));

  return 0;
}
