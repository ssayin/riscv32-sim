#ifndef RISCV32_SIM_TYPES_HPP
#define RISCV32_SIM_TYPES_HPP

#include <type_traits>

template <typename T>
concept Integral = std::is_integral<T>::value;

template <class T>
concept Unsigned = std::is_unsigned<T>::value;

template <typename T>
concept Enum = std::is_enum<T>::value;

#endif // RISCV32_SIM_TYPES_HPP