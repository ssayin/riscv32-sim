// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef LOADER_HPP
#define LOADER_HPP

#include <elfio/elfio.hpp>
#include <elfio/elfio_section.hpp>

namespace mem {
class address_router;
}

class loader {
private:
  ELFIO::elfio reader;

public:
  class loader_error : public std::runtime_error {
  public:
    loader_error(const std::string &message) : std::runtime_error(message) {}
  };

  explicit loader(const std::string &file_name);
  loader(const std::string &file_name, mem::address_router &mem);
  uint32_t                                    symbol(const std::string &str);
  uint32_t                                    entry();
  void                                        dump(std::ostream &out);
  std::vector<std::tuple<uint64_t, uint64_t>> progbit_ranges();
};

#endif // LOADER_HPP
