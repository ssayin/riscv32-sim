// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include "loader.hpp"
#include "memory/sparse_memory.hpp"

#include <elfio/elfio_dump.hpp>
#include <fmt/ostream.h>

loader::loader(const std::string &file_name) {
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
    fmt::print(stderr, "Loaded ELF does not target RISC-V");
    std::exit(EXIT_FAILURE);
  }

  if (reader.segments.size() == 0) {
    fmt::print(stderr, "Loaded ELF has no segments");
    std::exit(EXIT_FAILURE);
  }
}

loader::loader(const std::string &file_name, sparse_memory_accessor &mem)
    : loader{file_name} {
  std::for_each(reader.segments.begin(), reader.segments.end(),
                [&mem](std::unique_ptr<ELFIO::segment> &s) {
                  if (s->get_type() == ELFIO::PT_LOAD) {
                    auto addr = s->get_virtual_address();
                    auto size = s->get_file_size();
                    mem.write(addr, (void *)s->get_data(), size);
                  }
                });
}

uint32_t loader::symbol(const std::string &str) {
  const ELFIO::section                *sec = reader.sections[".symtab"];
  ELFIO::const_symbol_section_accessor symbols(reader, sec);
  ELFIO::Elf64_Addr                    value         = 0;
  ELFIO::Elf_Xword                     size          = 0;
  unsigned char                        bind          = 0;
  unsigned char                        type          = 0;
  ELFIO::Elf_Half                      section_index = 0;
  unsigned char                        other         = 0;
  if (!symbols.get_symbol(str, value, size, bind, type, section_index, other)) {
    throw std::runtime_error(
        fmt::format("unable to find '{}' symbol in the ELF file", str));
  }
  return value;
}

void loader::dump(std::ostream &out) {
  ELFIO::dump::header(out, reader);
  ELFIO::dump::section_headers(out, reader);
  ELFIO::dump::segment_headers(out, reader);
  ELFIO::dump::symbol_tables(out, reader);
  ELFIO::dump::notes(out, reader);
  ELFIO::dump::modinfo(out, reader);
  ELFIO::dump::dynamic_tags(out, reader);
  ELFIO::dump::section_datas(out, reader);
  ELFIO::dump::segment_datas(out, reader);
}
uint32_t loader::entry() { return reader.get_entry(); }
