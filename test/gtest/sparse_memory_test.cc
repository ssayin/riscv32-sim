#include "memory/sparse_memory.hpp"
#include "zicsr/sync_exception.hpp"
#include <fmt/format.h>
#include <gtest/gtest.h>

TEST(SparseMemoryTest, UninitializedMemory) {
  sparse_memory sm;
  EXPECT_THROW(sm.read_half(0x5), sync_exception);
}

TEST(SparseMemoryTest, Load) {
  uint8_t data[] = {0xFF, 0x34, 0xCC, 0x34};

  sparse_memory sm;
  sm.load(0x100, data, sizeof(data));
  EXPECT_EQ(sm.read_half(0x102), 0x34CC);
}
