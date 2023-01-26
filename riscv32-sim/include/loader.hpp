#ifndef LOADER_HPP
#define LOADER_HPP

#include "common/types.hpp"
#include "memory/sparse_memory.hpp"
#include <elfio/elfio.hpp>
#include <elfio/elfio_dump.hpp>
#include <fmt/printf.h>
#include <string>

class loader {
private:
  ELFIO::elfio reader;

public:
  explicit loader(const std::string &file_name, sparse_memory_accessor &mem);

  uint32_t symbol(const std::string &str);
  uint32_t entry() { return reader.get_entry(); }
  void     dump(std::ostream &out);
};

inline loader::loader(const std::string      &file_name,
                      sparse_memory_accessor &mem) {
  /*
   * Irrecoverable
   * Can resort to exceptions if necessary in the future.
   */
  if (!reader.load(file_name)) {
    fmt::print(stderr, "File is not an ELF file");
    std::exit(EXIT_FAILURE);
  }

  if (reader.get_class() != ELFIO::ELFCLASS32) {
    fmt::print(stderr, "Only 32-bit binaries are supported");
    std::exit(EXIT_FAILURE);
  }

  if (reader.get_machine() != ELFIO::EM_RISCV) {
    // std::cerr << "Loaded ELF does not target RISC-V";
    fmt::print(stderr, "Loaded ELF does not target RISC-V");
    std::exit(EXIT_FAILURE);
  }

  if (reader.segments.size() == 0) {
    fmt::print(stderr, "Loaded ELF has no segments");
    std::exit(EXIT_FAILURE);
  }

  // dump();

  std::for_each(reader.segments.begin(), reader.segments.end(),
                [&mem](std::unique_ptr<ELFIO::segment> &s) {
                  if (s->get_type() == ELFIO::PT_LOAD) {
                    auto addr = s->get_virtual_address();
                    auto size = s->get_file_size();
                    mem.write(addr, (void *)s->get_data(), size);
                  }
                });
}

#endif // LOADER_HPP
