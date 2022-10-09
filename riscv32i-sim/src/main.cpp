#include <cstdio>

#include <elfio/elfio.hpp>
#include <elfio/elfio_dump.hpp>
#include <elfio/elfio_segment.hpp>

#include "Defs.hpp"

#include <algorithm>

#include "rv32_decode.hpp"

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

  ELFIO::Elf_Half n = reader.segments.size();
  if (n == 0) {
    return 1;
  }

  ELFIO::dump::header(std::cout, reader);
  ELFIO::dump::section_headers(std::cout, reader);
  ELFIO::dump::segment_headers(std::cout, reader);
  ELFIO::dump::symbol_tables(std::cout, reader);
  ELFIO::dump::notes(std::cout, reader);
  ELFIO::dump::modinfo(std::cout, reader);
  ELFIO::dump::dynamic_tags(std::cout, reader);
  ELFIO::dump::section_datas(std::cout, reader);
  ELFIO::dump::segment_datas(std::cout, reader);

  decoder_out o = decode(0x30);
  std::cout << std::boolalpha << o.has_imm << std::endl;
  /*
    Computer c;

    std::ranges::for_each(
        reader.segments, [&](std::unique_ptr<ELFIO::segment> &s) {
          if (s->get_type() == ELFIO::PT_LOAD) {
            std::uint64_t addr = s->get_virtual_address();
            std::uint64_t size = s->get_file_size();
            if ((addr + size) < c.mem.MemSize)
              std::memcpy((void *)(c.Mem + addr), s->get_data(), size);
          }
        });

    c.PC = reader.get_entry();

    printf("Starting Simulation:\n");

    for (int i = 0; i < 100; ++i) {
      c.step();
    }
  */
  return 0;
}
