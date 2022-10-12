#include "loader.hpp"
#include "iss_model.hpp"
#include <elfio/elfio.hpp>
#include <elfio/elfio_dump.hpp>
#include <elfio/elfio_section.hpp>
#include <elfio/elfio_segment.hpp>

#include <algorithm>
#include <cstdio>

ELFIO::Elf64_Addr tohost_addr(ELFIO::elfio &reader) {
  const ELFIO::section                *sec = reader.sections[".symtab"];
  ELFIO::const_symbol_section_accessor symbols(reader, sec);
  ELFIO::Elf64_Addr                    value = 0;
  ELFIO::Elf_Xword                     size  = 0;
  unsigned char                        bind  = 0;
  unsigned char                        type  = 0;
  ELFIO::Elf_Half                      section_index;
  unsigned char                        other;
  if (!symbols.get_symbol("tohost", value, size, bind, type, section_index,
                          other)) {

    throw std::runtime_error("unable to find 'tohost' symbol in the ELF file");
  }
  return value;
}

void loader::load(std::string file_name, iss_model &model) {
  ELFIO::elfio reader;

  if (!reader.load(file_name)) {
    throw std::runtime_error("File is not found or it is not an ELF file\n");
  }

  if (reader.get_class() != ELFIO::ELFCLASS32) {
    throw std::runtime_error("Only 32-bit binaries are supported");
  }

  if (reader.get_machine() != ELFIO::EM_RISCV) {
    throw std::runtime_error("Loaded ELF does not target RISC-V");
  }

  if (reader.segments.size() == 0) {
    throw std::runtime_error("Loaded ELF has no segments");
  }

  std::for_each(reader.segments.begin(), reader.segments.end(),
                [&](std::unique_ptr<ELFIO::segment> &s) {
                  if (s->get_type() == ELFIO::PT_LOAD) {
                    model.load(s->get_virtual_address(), (void *)s->get_data(),
                               s->get_file_size());
                  }
                });

  model.PC          = reader.get_entry();
  model.tohost_addr = tohost_addr(reader);
}
