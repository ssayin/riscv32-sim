// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef COMMON_HART_STATE_HPP
#define COMMON_HART_STATE_HPP

#include "op.hpp"
#include "program_counter.hpp"

struct csr_change {
  uint16_t index;
  uint32_t prev;
  uint32_t next;
};

struct gpr_change {
  uint8_t  index;
  uint32_t prev;
  uint32_t next;
};

struct hart_state {
  op                      dec;
  program_counter         pc;
  uint32_t                instr;
  std::vector<csr_change> csr_staged;
  std::vector<gpr_change> gpr_staged;
  hart_state(uint32_t pc) : pc{pc} {}
};

#endif /* end of include guard: COMMON_HART_STATE_HPP */
