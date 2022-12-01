#ifndef RISCV32_SIM_LOADER_HPP
#define RISCV32_SIM_LOADER_HPP

#include "common/types.hpp"
#include <elfio/elfio.hpp>
#include <string>

class loader {
  ELFIO::elfio reader;

public:
  template <Memory_Model MM>
  explicit loader(const std::string &file_name, MM &mem);
  uint32_t symbol(const std::string &str);
  uint32_t entry() { return reader.get_entry(); }
};

template <Memory_Model MM>
loader::loader(const std::string &file_name, MM &mem) {
  if (!reader.load(file_name))
    throw std::runtime_error("File is not found or it is not an ELF file\n");

  if (reader.get_class() != ELFIO::ELFCLASS32)
    throw std::runtime_error("Only 32-bit binaries are supported");

  if (reader.get_machine() != ELFIO::EM_RISCV)
    throw std::runtime_error("Loaded ELF does not target RISC-V");

  if (reader.segments.size() == 0)
    throw std::runtime_error("Loaded ELF has no segments");

  std::for_each(reader.segments.begin(), reader.segments.end(),
                [&mem](std::unique_ptr<ELFIO::segment> &s) {
                  if (s->get_type() == ELFIO::PT_LOAD) {
                    mem.load(s->get_virtual_address(), (void *)s->get_data(),
                             s->get_file_size());
                  }
                });
}

#endif // RISCV32_SIM_LOADER_HPP
