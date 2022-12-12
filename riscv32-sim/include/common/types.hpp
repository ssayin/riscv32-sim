#ifndef RISCV32_SIM_TYPES_HPP
#define RISCV32_SIM_TYPES_HPP

#include <cstdint>
#include <type_traits>

template <typename T>
concept Integral = std::is_integral<T>::value;

template <class T>
concept Unsigned = std::is_unsigned<T>::value;

template <typename T>
concept Enum = std::is_enum<T>::value;

template <typename T>
concept Memory_Model = requires(T mm, uint32_t addr, void *ptr, int64_t size) {
  mm.load(addr, ptr, size);
};

#endif // RISCV32_SIM_TYPES_HPP