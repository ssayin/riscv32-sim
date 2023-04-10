// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include "memory/sparse_memory.hpp"

#include <gtest/gtest.h>

TEST(SparseMemoryTest, UninitializedMemory) {
  mem::sparse_memory sm;
  // EXPECT_THROW(sm.read_half(0x5), sync_exception);
}

TEST(SparseMemoryTest, Load) {
  uint8_t data[] = {0xFF, 0x34, 0xCC, 0x34};

  mem::sparse_memory            smem;
  mem::sparse_memory_accessor<> acc{smem};

  acc.write(0x100, data, sizeof(data));
  EXPECT_EQ(acc.read16(0x102), 0x34CC);
}
