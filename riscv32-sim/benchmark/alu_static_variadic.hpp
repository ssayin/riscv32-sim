// SPDX-FileCopyrightText: 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef BENCHMARK_ALU_STATIC_VARIADIC_HPP
#define BENCHMARK_ALU_STATIC_VARIADIC_HPP

#include "arith.hpp"
#include "common/common.hpp"

#include <limits>
#include <stdexcept>

template <alu Opt> struct alu_function;

template <> struct alu_function<alu::_or> {
  static inline uint32_t exec(uint32_t a, uint32_t b) { return a | b; }
};

template <> struct alu_function<alu::_xor> {
  static inline uint32_t exec(uint32_t a, uint32_t b) { return a ^ b; }
};

template <> struct alu_function<alu::_add> {
  static inline uint32_t exec(uint32_t a, uint32_t b) { return a + b; }
};

template <> struct alu_function<alu::_auipc> {
  static inline uint32_t exec(uint32_t a, uint32_t b) { return a + b; }
};

template <> struct alu_function<alu::_jal> {
  static inline uint32_t exec(uint32_t a, uint32_t b) { return a + b; }
};

template <> struct alu_function<alu::_sub> {
  static inline uint32_t exec(uint32_t a, uint32_t b) { return a - b; }
};

template <> struct alu_function<alu::_sll> {
  static inline uint32_t exec(uint32_t a, uint32_t b) { return a << b; }
};

template <> struct alu_function<alu::_srl> {
  static inline uint32_t exec(uint32_t a, uint32_t b) { return a >> b; }
};

template <> struct alu_function<alu::_sra> {
  static inline uint32_t exec(uint32_t a, uint32_t b) {
    return static_cast<int32_t>(a) >> b;
  }
};

template <> struct alu_function<alu::_slt> {
  static inline uint32_t exec(uint32_t a, uint32_t b) {
    return static_cast<uint32_t>(static_cast<int32_t>(a) <
                                 static_cast<int32_t>(b));
    ;
  }
};

template <> struct alu_function<alu::_sltu> {
  static inline uint32_t exec(uint32_t a, uint32_t b) {
    return static_cast<uint32_t>(a < b);
  }
};

template <> struct alu_function<alu::_jalr> {
  static inline uint32_t exec(uint32_t a, uint32_t b) {
    return (a + b) & masks::lsb_zero;
  }
};

template <> struct alu_function<alu::_mulh> {
  static inline uint32_t exec(uint32_t a, uint32_t b) {
    return mulh(static_cast<int32_t>(a), static_cast<int32_t>(b));
  }
};

template <> struct alu_function<alu::_mulhsu> {
  static inline uint32_t exec(uint32_t a, uint32_t b) {
    return mulhsu(static_cast<int32_t>(a), b);
  }
};

template <> struct alu_function<alu::_mulhu> {
  static inline uint32_t exec(uint32_t a, uint32_t b) { return mulhu(a, b); }
};

template <> struct alu_function<alu::_mul> {
  static inline uint32_t exec(uint32_t a, uint32_t b) {
    return static_cast<int32_t>(a) * static_cast<int32_t>(b);
  }
};

template <> struct alu_function<alu::_and> {
  static inline uint32_t exec(uint32_t a, uint32_t b) { return a & b; }
};

template <> struct alu_function<alu::_divu> {
  static inline uint32_t exec(uint32_t a, uint32_t b) {
    if (b == 0) return std::numeric_limits<uint32_t>::max();
    return a / b;
  }
};

template <> struct alu_function<alu::_remu> {
  static inline uint32_t exec(uint32_t a, uint32_t b) {
    if (b == 0) return a;
    return a % b;
  }
};

template <> struct alu_function<alu::_rem> {
  static inline uint32_t exec(uint32_t a, uint32_t b) {
    auto dividend = static_cast<int32_t>(a);
    auto divisor  = static_cast<int32_t>(b);
    if (divisor == 0) return dividend;
    if ((dividend == -2147483648) && (divisor == -1)) return 0;
    return dividend % divisor;
  }
};

// ... more functions ...

template <> struct alu_function<alu::_div> {
  static inline uint32_t exec(uint32_t a, uint32_t b) {
    auto dividend = static_cast<int32_t>(a);
    auto divisor  = static_cast<int32_t>(b);
    if (divisor == 0) return -1;
    if ((dividend == -2147483648) && (divisor == -1)) return dividend;
    return dividend / divisor;
  }
};

template <alu Opt> inline uint32_t do_alu_op(uint32_t opd_1, uint32_t opd_2) {
  return alu_function<Opt>::exec(opd_1, opd_2);
}

template <alu Opt> inline bool is_alu_op(alu op) { return op == Opt; }

template <alu... Ops>
constexpr uint32_t do_alu_switch(enum alu opt, uint32_t opd_1, uint32_t opd_2) {
  static_assert(sizeof...(Ops) > 0, "at least one op must be provided");
  if ((is_alu_op<Ops>(opt) || ...)) {
    return ((opt == Ops ? do_alu_op<Ops>(opd_1, opd_2) : 0) | ...);
  } else {
    throw std::runtime_error("asd");
  }
}

template <>
inline uint32_t do_alu_switch<>(enum alu opt, uint32_t opd_1, uint32_t opd_2) {
  throw std::runtime_error("asd");
}

inline uint32_t do_alu_static(enum alu opt, uint32_t opd_1, uint32_t opd_2) {
  return do_alu_switch<alu::_or, alu::_and, alu::_xor, alu::_add, alu::_auipc,
                       alu::_jal, alu::_sub, alu::_sll, alu::_srl, alu::_sra,
                       alu::_slt, alu::_sltu, alu::_jalr, alu::_mulh,
                       alu::_mulhsu, alu::_mulhu, alu::_mul, alu::_divu,
                       alu::_remu, alu::_div, alu::_rem>(opt, opd_1, opd_2);
}

#endif /* end of include guard: BENCHMARK_ALU_STATIC_VARIADIC_HPP */
