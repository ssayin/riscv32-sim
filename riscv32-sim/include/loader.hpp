#ifndef LOADER_HPP
#define LOADER_HPP

#include "common/types.hpp"
#include <elfio/elfio.hpp>
#include <elfio/elfio_dump.hpp>
#include <fmt/printf.h>
#include <string>

class loader {
private:
  ELFIO::elfio reader;

public:
  template <MemoryModel Model>
  explicit loader(const std::string &file_name, Model &mem);

  uint32_t symbol(const std::string &str);
  uint32_t entry() { return reader.get_entry(); }
  void     dump(std::ostream &out);
};

template <MemoryModel Model>
loader::loader(const std::string &file_name, Model &mem) {
  if (!reader.load(file_name))
    throw std::runtime_error("File is not found or it is not an ELF file");

  if (reader.get_class() != ELFIO::ELFCLASS32)
    throw std::runtime_error("Only 32-bit binaries are supported");

  if (reader.get_machine() != ELFIO::EM_RISCV)
    throw std::runtime_error("Loaded ELF does not target RISC-V");

  if (reader.segments.size() == 0)
    throw std::runtime_error("Loaded ELF has no segments");

  // dump();

  std::for_each(reader.segments.begin(), reader.segments.end(),
                [&mem](std::unique_ptr<ELFIO::segment> &s) {
                  if (s->get_type() == ELFIO::PT_LOAD) {
                    auto addr = s->get_virtual_address();
                    auto size = s->get_file_size();
                    mem.load(addr, (void *)s->get_data(), size);
                  }
                });
}

#endif // LOADER_HPP
