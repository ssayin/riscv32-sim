#include "loader.hpp"
#include "fmt/format.h"
#include <elfio/elfio.hpp>
#include <elfio/elfio_section.hpp>

uint32_t loader::symbol(const std::string &str) {
  const ELFIO::section                *sec = reader.sections[".symtab"];
  ELFIO::const_symbol_section_accessor symbols(reader, sec);
  ELFIO::Elf64_Addr                    value = 0;
  ELFIO::Elf_Xword                     size  = 0;
  unsigned char                        bind  = 0;
  unsigned char                        type  = 0;
  ELFIO::Elf_Half                      section_index;
  unsigned char                        other;
  if (!symbols.get_symbol(str, value, size, bind, type, section_index, other)) {
    throw std::runtime_error(
        fmt::format("unable to find '{}' symbol in the ELF file", str));
  }
  return value;
}
