// SPDX-FileCopyrightText: 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include "arith.hpp"
#include "common/common.hpp"
#include <cstdint>
#include <limits>
#include <stdexcept>

uint32_t do_alu(enum alu opt, uint32_t opd_1, uint32_t opd_2) {

  auto is_overflow = [](int32_t dividend, int32_t divisor) {
    return (dividend == -2147483648) && (divisor == -1);
  };

  switch (opt) {
  case alu::_or:
    return opd_1 | opd_2;

  case alu::_and:
    return opd_1 & opd_2;

  case alu::_xor:
    return opd_1 ^ opd_2;

  case alu::_add:
  case alu::_auipc:
  case alu::_jal:
    return opd_1 + opd_2;

  case alu::_sub:
    return opd_1 - opd_2;

  case alu::_sll:
    return opd_1 << opd_2;

  case alu::_srl:
    return opd_1 >> opd_2;

  case alu::_sra:
    return static_cast<int32_t>(opd_1) >> opd_2;

  case alu::_slt:
    return static_cast<uint32_t>(static_cast<int32_t>(opd_1) <
                                 static_cast<int32_t>(opd_2));

  case alu::_sltu:
    return static_cast<uint32_t>(opd_1 < opd_2);

  case alu::_jalr:
    return (opd_1 + opd_2) & masks::lsb_zero;

    /*
     * Extension M
     */

  case alu::_mulh:
    return mulh(static_cast<int32_t>(opd_1), static_cast<int32_t>(opd_2));

  case alu::_mulhsu:
    return mulhsu(static_cast<int32_t>(opd_1), opd_2);

  case alu::_mulhu:
    return mulhu(opd_1, opd_2);

  case alu::_mul:
    return static_cast<int32_t>(opd_1) * static_cast<int32_t>(opd_2);

    /*
 clang-format off
+------------------------+-----------+----------+----------+----------+
|       Condition        |  Dividend |  Divisor |  DIVU[W] |  REMU[W] |
+------------------------+-----------+----------+----------+----------+
| Division by zero       |  x        |  0       |  2^L − 1 |  x       |
+------------------------+-----------+----------+----------+----------+
clang-format on
  */

  case alu::_divu:
    if (opd_2 == 0) return std::numeric_limits<uint32_t>::max();
    return opd_1 / opd_2;

  case alu::_remu:
    if (opd_2 == 0) return opd_1;
    return opd_1 % opd_2;

    /*
 clang-format off
+------------------------+-----------+----------+-----------+---------+
|       Condition        |  Dividend |  Divisor |   DIV[W]  |  REM[W] |
+------------------------+-----------+----------+-----------+---------+
| Division by zero       |  x        |  0       |  −1       |  x      |
| Overflow (signed only) |  −2^{L−1} |  −1      |  −2^{L−1} |  0      |
+------------------------+-----------+----------+-----------+---------+
clang-format on
 */

  case alu::_div: {
    auto dividend = static_cast<int32_t>(opd_1);
    auto divisor  = static_cast<int32_t>(opd_2);
    if (divisor == 0) return -1;
    if (is_overflow(dividend, divisor)) return dividend;
    return dividend / divisor;
  }

  case alu::_rem: {
    auto dividend = static_cast<int32_t>(opd_1);
    auto divisor  = static_cast<int32_t>(opd_2);
    if (divisor == 0) return dividend;
    if (is_overflow(dividend, divisor)) return 0;
    return dividend % divisor;
  }
  default:
    throw std::runtime_error("asd");
    // throw model::unexpected_switch_case_value();
  }
}
