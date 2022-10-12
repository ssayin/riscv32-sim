#include <cstdio>

#include <elfio/elfio.hpp>
#include <elfio/elfio_dump.hpp>
#include <elfio/elfio_section.hpp>
#include <elfio/elfio_segment.hpp>

#include <algorithm>

#include "iss_model.hpp"
#include "rv32_decode.hpp"

void load_tohost_addr(ELFIO::elfio &reader, iss_model &model) {
  const ELFIO::section                *sec = reader.sections[".symtab"];
  ELFIO::const_symbol_section_accessor symbols(reader, sec);
  ELFIO::Elf64_Addr                    value = 0;
  ELFIO::Elf_Xword                     size  = 0;
  unsigned char                        bind  = 0;
  unsigned char                        type  = 0;
  ELFIO::Elf_Half                      section_index;
  unsigned char                        other;
  if (symbols.get_symbol("tohost", value, size, bind, type, section_index,
                         other)) {
    model.tohost_addr = value;
  } else {
    throw std::runtime_error("unable to find 'tohost' symbol in the ELF file");
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: riscv32i-sim <file_name>\n");
    return 1;
  }

  ELFIO::elfio reader;

  if (!reader.load(argv[1])) {
    printf("File %s is not found or it is not an ELF file\n", argv[1]);
    return 1;
  }

  if (reader.get_class() != ELFIO::ELFCLASS32) {
    printf("Only 32-bit binaries are supported\n");
    return 1;
  }

  if (reader.segments.size() == 0) {
    return 1;
  }

  iss_model model;
  std::for_each(reader.segments.begin(), reader.segments.end(),
                [&](std::unique_ptr<ELFIO::segment> &s) {
                  if (s->get_type() == ELFIO::PT_LOAD) {
                    model.load_program(s->get_virtual_address(),
                                       (void *)s->get_data(),
                                       s->get_file_size());
                  }
                });

  model.PC = reader.get_entry();
  load_tohost_addr(reader, model);

  while (!model.done()) {
    model.step();
  }

  return 0;
}
