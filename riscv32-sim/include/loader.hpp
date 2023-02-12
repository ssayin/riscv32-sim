#ifndef LOADER_HPP
#define LOADER_HPP

#include <elfio/elfio.hpp>

class sparse_memory_accessor;

class loader {
private:
  ELFIO::elfio reader;

public:
  explicit loader(const std::string &file_name);
  loader(const std::string &file_name, sparse_memory_accessor &mem);
  uint32_t symbol(const std::string &str);
  uint32_t entry();
  void     dump(std::ostream &out);
};

#endif // LOADER_HPP
