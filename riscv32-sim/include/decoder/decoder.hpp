#ifndef DECODER_DECODER_HPP
#define DECODER_DECODER_HPP

#include "rv32_isn.hpp"
#include <cstdint>
#include <nlohmann/json.hpp>
#include <variant>

static constexpr uint32_t ecall  = 0x73U;
static constexpr uint32_t ebreak = 0x9002U;
static constexpr uint32_t mret   = 0x30200073U;
static constexpr uint32_t sret   = 0x10200073U;
static constexpr uint32_t wfi    = 0x10500073U;

enum class target : uint8_t {
  load,
  store,
  alu,
  branch,
  csr,
  mret,
  illegal,
  ebreak,
  ecall
};

enum class alu : uint8_t {
  _and,
  _or,
  _xor,
  _add,
  _sub,
  _sll,
  _srl,
  _sra,
  _slt,
  _sltu,
  _mul,
  _mulh,
  _mulhsu,
  _mulhu,
  _div,
  _divu,
  _rem,
  _remu,
  _auipc,
  _jal,
  _jalr,
};

using op_type = std::variant<std::monostate, alu, masks::load, masks::store,
                             masks::branch, masks::sys>;

template <typename... Ts> struct overload : Ts... {
  using Ts::operator()...;
};

template <class... Ts> overload(Ts...) -> overload<Ts...>;

namespace nlohmann {
template <> struct adl_serializer<op_type> {
  static void from_json(const json &j, op_type &type) {
    auto x = j.get<uint8_t>();
    if (x == 0) type = std::monostate{};
    if (x >= 1 && x <= 19) {
      type = masks::sys{--x};
    }
    if (x >= 20 && x <= 29) type = masks::branch{x -= 20};
    if (x >= 30 && x <= 39) type = masks::store{x -= 30};
    if (x >= 40 && x <= 49) type = masks::load{x -= 40};
    if (x >= 50 && x <= 100) type = alu{x -= 50};
    throw std::runtime_error("cannot parse");
  }
  static void to_json(json &j, const op_type &opt) {
    j = std::visit(
        overload{
            [](const alu &a) { return 50 + static_cast<uint8_t>(a); },
            [](const masks::load &l) { return 40 + static_cast<uint8_t>(l); },
            [](const masks::store &s) { return 30 + static_cast<uint8_t>(s); },
            [](const masks::branch &b) { return 20 + static_cast<uint8_t>(b); },
            [](const masks::sys &s) { return 1 + static_cast<uint8_t>(s); },
            [](const std::monostate &) { return 0; }},
        opt);
  }
};
} // namespace nlohmann

struct op {
  uint32_t imm;
  op_type  opt; // 2 bytes
  target   tgt;
  uint8_t  rd;
  uint8_t  rs1;
  uint8_t  rs2;
  bool     has_imm;
  bool     use_pc = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(op, imm, opt, tgt, rd, rs1, rs2, rs2,
                                   has_imm, use_pc)

op decode(uint32_t word);

#endif // DECODER_DECODER_HPP
