#ifndef ZICSR_PRIVILEGE_LEVEL_HPP
#define ZICSR_PRIVILEGE_LEVEL_HPP

enum class privilege {
  user       = 0b00,
  supervisor = 0b01,
  hypervisor = 0b10,
  machine    = 0b11
};

#endif // ZICSR_PRIVILEGE_LEVEL_HPP
