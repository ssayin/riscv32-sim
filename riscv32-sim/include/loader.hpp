#ifndef LOADER_HPP
#define LOADER_HPP

#include "common/types.hpp"
#include <elfio/elfio.hpp>
#include <elfio/elfio_dump.hpp>
#include <fmt/printf.h>
#include <string>

class loader {
  ELFIO::elfio reader;

public:
  template <MemoryModel Model>
  explicit loader(const std::string &file_name, Model &mem);

  uint32_t symbol(const std::string &str);
  uint32_t entry() { return reader.get_entry(); }
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

  /* if constexpr (Dump) {
     ELFIO::dump::header(std::cout, reader);
     ELFIO::dump::section_headers(std::cout, reader);
     ELFIO::dump::segment_headers(std::cout, reader);
     ELFIO::dump::symbol_tables(std::cout, reader);
     ELFIO::dump::notes(std::cout, reader);
     ELFIO::dump::modinfo(std::cout, reader);
     ELFIO::dump::dynamic_tags(std::cout, reader);
     ELFIO::dump::section_datas(std::cout, reader);
     ELFIO::dump::segment_datas(std::cout, reader);

     std::for_each(reader.sections.begin(), reader.sections.end(),
                   [&](std::unique_ptr<ELFIO::section> &s) {
                     fmt::print("{} {}\n", s->get_name(), s->get_flags());
                   });

     const ELFIO::section                *sec = reader.sections[".symtab"];
     ELFIO::const_symbol_section_accessor symbols(reader, sec);

     for (int i = 0; i < symbols.get_symbols_num(); ++i) {
     }
   } */

  std::for_each(reader.segments.begin(), reader.segments.end(),
                [&mem](std::unique_ptr<ELFIO::segment> &s) {
                  if (s->get_type() == ELFIO::PT_LOAD) {
                    mem.load(s->get_virtual_address(), (void *)s->get_data(),
                             s->get_file_size());
                  }
                });
}

#endif // LOADER_HPP
