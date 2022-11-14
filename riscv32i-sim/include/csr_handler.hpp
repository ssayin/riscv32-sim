#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string_view>

enum class csr : uint16_t {
  ustatus = 0x000,
  fflags  = 0x001,
  frm     = 0x002,
  fcsr    = 0x003,
  uie     = 0x004,
  utvec   = 0x005,

  uscratch = 0x040,
  uepc     = 0x041,
  ucause   = 0x042,
  utval    = 0x043,
  uip      = 0x044,

  cycle         = 0xC00,
  time          = 0xC01,
  instret       = 0xC02,
  hpmcounter3   = 0xC03,
  hpmcounter31  = 0xC1F,
  cycleh        = 0xC80,
  timeh         = 0xC81,
  instreth      = 0xC82,
  hpmcounter3h  = 0xC83,
  hpmcounter31h = 0xC9F,

  sstatus    = 0x100,
  sedeleg    = 0x102,
  sideleg    = 0x103,
  sie        = 0x104,
  stvec      = 0x105,
  scounteren = 0x106,

  sscratch = 0x140,
  sepc     = 0x141,
  scause   = 0x142,
  stval    = 0x143,
  sip      = 0x144,

  satp = 0x180,

  mvendorid  = 0xF11,
  marchid    = 0xF12,
  mimpid     = 0xF13,
  mhartid    = 0xF14,
  mstatus    = 0x300,
  misa       = 0x301,
  medeleg    = 0x302,
  mideleg    = 0x303,
  mie        = 0x304,
  mtvec      = 0x305,
  mcounteren = 0x306,

  mscratch = 0x340,
  mepc     = 0x341,
  mcause   = 0x342,
  mtval    = 0x343,
  mip      = 0x344,

  pmpcfg0   = 0x3A0,
  pmpcfg3   = 0x3A3,
  pmpaddr0  = 0x3B0,
  pmpaddr15 = 0x3BF,

  mcycle         = 0xB00,
  minstret       = 0xB02,
  mhpmcounter3   = 0xB03,
  mhmpmcounter31 = 0xB1F,

  mcycleh        = 0xB80,
  minstreth      = 0xB82,
  mhpmcounter3h  = 0xB83,
  mhpmcounter31h = 0xB9F,

  mhpmevent3  = 0x323,
  mhpmevent31 = 0x33F,

  tselect = 0x7A0,
  tdata1  = 0x7A1,
  tdata2  = 0x7A2,
  tdata3  = 0x7A3,

  dcsr     = 0x7B0,
  dpc      = 0x7B1,
  dscratch = 0x7B2,
};

consteval uint32_t operator"" _MISA(const char *isa, size_t) {
  std::string_view str{isa};
  auto             sel = [](uint32_t acc, char c) -> uint32_t {
    return acc | ((c >= 'A' && c <= 'Z') ? 1 << (25 - ('Z' - c)) : 0);
  };
  return std::accumulate(str.begin(), str.end(), 0x0, sel);
}

constexpr uint32_t RV32 = (1 << 30);

class csr_handler {
  static bool                is_unimplemented_csr(uint32_t addr);
  std::array<uint32_t, 4096> csrs{};

public:
  csr_handler();
  uint32_t read(uint32_t addr);
  void     write(uint32_t addr, uint32_t v);
  void     write(csr addr, uint32_t v);
  uint32_t read(csr addr);

  enum priv {
    user [[maybe_unused]] = 0,
    supervisor [[maybe_unused]],
    reserved [[maybe_unused]],
    machine
  };
  priv mode = machine;
};
