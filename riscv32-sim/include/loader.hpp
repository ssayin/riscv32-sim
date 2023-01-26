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

#endif // LOADER_HPP
