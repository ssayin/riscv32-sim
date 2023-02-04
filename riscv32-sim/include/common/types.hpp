#ifndef COMMON_TYPES_HPP
#define COMMON_TYPES_HPP

#include "masks.hpp"

#include <string>
#include <type_traits>
#include <variant>

template <class T>
concept UnsignedIntegral =
    std::is_unsigned<T>::value && std::is_integral<T>::value;

template <typename T>
concept Enum = std::is_enum<T>::value;

template <typename T>
concept MemoryModel = requires(T mem, uint32_t addr, void *ptr, int64_t size) {
                        mem.write(addr, ptr, size);
                      };

constexpr auto to_int(Enum auto val) {
  return static_cast<std::underlying_type_t<decltype(val)>>(val);
}

struct options {
  std::string tohost_sym = "tohost";
  uint32_t    mtime{};
  uint32_t    mtimecmp{};
  uint32_t    interval{};
  bool        trace       = false;
  bool        mti_enabled = false;
};

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
struct op {
  uint32_t imm;
  op_type  opt; // 2 bytes
  target   tgt;
  uint8_t  rd;
  uint8_t  rs1;
  uint8_t  rs2;
  bool     has_imm;
  bool     use_pc        = false;
  bool     is_compressed = false;
};

struct state {
  op       dec;
  uint32_t instr;
};

#endif // COMMON_TYPES_HPP
