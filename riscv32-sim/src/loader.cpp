// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include "loader.hpp"
#include "memory/sparse_memory.hpp"

#include <elfio/elfio_dump.hpp>

#include "spdlog/fmt/bundled/core.h"

loader::loader(const std::string &file_name) {
  if (!reader.load(file_name)) {
    throw loader_error("File is not an ELF file");
  }

  if (reader.get_class() != ELFIO::ELFCLASS32) {
    throw loader_error("Only 32-bit binaries are supported");
  }

  if (reader.get_machine() != ELFIO::EM_RISCV) {
    throw loader_error("Loaded ELF does not target RISC-V");
  }

  if (reader.segments.size() == 0) {
    throw loader_error("Loaded ELF has no segments");
  }
}

loader::loader(const std::string &file_name, mem::address_router &mem)
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

std::vector<std::tuple<uint64_t, uint64_t>> loader::progbit_ranges() {
  std::vector<std::tuple<uint64_t, uint64_t>> ranges;
  for (const auto &section : reader.sections) {
    if (section->get_type() == ELFIO::SHT_PROGBITS &&
        (section->get_flags() & ELFIO::SHF_EXECINSTR) &&
        section->get_size() > 0) {
      uint64_t start_addr = section->get_address();
      uint64_t end_addr   = start_addr + section->get_size() - 1;
      ranges.emplace_back(start_addr, end_addr);
    }
  }
  return ranges;
}
