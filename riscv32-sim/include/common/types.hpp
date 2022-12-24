#ifndef COMMON_TYPES_HPP
#define COMMON_TYPES_HPP

#include <cstdint>
#include <type_traits>

template <class T>
concept Unsigned = std::is_unsigned<T>::value;

template <typename T>
concept Enum = std::is_enum<T>::value;

template <typename T>
concept Memory_Model = requires(T mem, uint32_t addr, void *ptr, int64_t size) {
  mem.load(addr, ptr, size);
};

#endif // COMMON_TYPES_HPP
