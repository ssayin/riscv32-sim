// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <cstdint>
#include <string>

#include "config.hpp"

struct options {
  std::string target;
  std::string tohost_sym = "tohost";
  uint32_t    mtime{};
  uint32_t    mtimecmp{};
  uint32_t    interval{};
#ifdef ENABLE_TCP
  uint16_t port;
  bool     tcp_enabled = false;
#endif
  bool trace       = false;
  bool export_json = false;
  bool mti_enabled = false;
  bool fstep       = false;
  bool dump_exit   = false;
};

#endif /* end of include guard: OPTIONS_HPP */
