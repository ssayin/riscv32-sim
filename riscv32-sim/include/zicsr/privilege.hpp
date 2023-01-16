#ifndef ZICSR_PRIVILEGE_LEVEL_HPP
#define ZICSR_PRIVILEGE_LEVEL_HPP

enum class privilege {
  user       = 0b00,
  supervisor = 0b01,
  hypervisor = 0b10,
  machine    = 0b11
};


struct privileged_mode {

  privilege mode;

   operator privilege() { return mode; }
   //operator uint16_t() { return static_cast<uint16_t>(mode); }
   //operator int16_t() { return static_cast<int16_t>(mode); }


  uint8_t priv(uint32_t addr) { return offset(addr, 8U, 9U); };

  bool is_write_legal_csr(uint32_t addr) {
    return !(is_readonly(addr) || (priv(addr) > to_int(mode)));
  }
  bool is_read_legal_csr(uint32_t addr) {
    return !(priv(addr) > to_int(mode));
  }

  bool is_readonly(uint32_t addr) { return offset(addr, 10U, 11U) == 0b11; };


  uint16_t priv_base() const {
    return static_cast<uint16_t>(static_cast<uint16_t>(mode) << 8);
  }

  uint16_t priv_csr(enum csr c) const { return priv_base() | to_int(c); }
};

#endif // ZICSR_PRIVILEGE_LEVEL_HPP
