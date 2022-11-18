#ifndef RISCV32_SIM_LOADER_HPP
#define RISCV32_SIM_LOADER_HPP

#include <string>

class iss_model;

struct loader {
  static void load(std::string file_name, iss_model &model);
};

#endif // RISCV32_SIM_LOADER_HPP
