#ifndef MEMORY_SPARSE_MEMORY_HPP
#define MEMORY_SPARSE_MEMORY_HPP

#include <cstdint>
#include <memory>
#include <unordered_map>

using sparse_memory = std::unordered_map<uint32_t, std::unique_ptr<uint8_t[]>>;

class sparse_memory_accessor {
  uint32_t ensure_page_exists(uint32_t addr);

  sparse_memory &page;

public:
  sparse_memory_accessor(const sparse_memory_accessor &)            = default;
  sparse_memory_accessor(sparse_memory_accessor &&)                 = delete;
  sparse_memory_accessor &operator=(const sparse_memory_accessor &) = delete;
  sparse_memory_accessor &operator=(sparse_memory_accessor &&)      = delete;
  explicit sparse_memory_accessor(sparse_memory &page) : page{page} {}
  virtual ~sparse_memory_accessor()   = default;
  constexpr static uint32_t page_size = 4096U;
  constexpr static uint32_t mask      = 0xFFFFF000U;

  void write(uint32_t virt_addr, void *ptr, int64_t size_in_bytes);

  [[nodiscard]] virtual uint8_t  read8(uint32_t off) const;
  [[nodiscard]] virtual uint16_t read16(uint32_t off) const;
  [[nodiscard]] virtual uint32_t read32(uint32_t off) const;
  [[nodiscard]] virtual uint64_t read64(uint32_t off) const;
  virtual void                   write8(uint32_t off, uint8_t b);
  virtual void                   write16(uint32_t off, uint16_t h);
  virtual void                   write32(uint32_t off, uint32_t w);
  virtual void                   write64(uint32_t off, uint64_t dw);
};

#endif // MEMORY_SPARSE_MEMORY_HPP
