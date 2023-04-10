// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef MODEL_TEST_H
#define MODEL_TEST_H

#define XLEN 32

#define TEST_CASE_1

#define RVMODEL_BOOT                                                           \
  .option push;                                                                \
  .option norelax;                                                             \
  la      gp, __global_pointer$;                                               \
  .option pop;                                                                 \
  la      sp, _sp;                                                             \
  la      t0, _start_trap_handler;                                             \
  csrw    mtvec, t0;

#define RVMODEL_DATA_BEGIN                                                     \
  .globl tohost;                                                               \
  .type  tohost, @object;                                                      \
  .size  tohost, 4;                                                            \
  tohost:                                                                      \
  .zero 4;                                                                     \
  .align 4;                                                                    \
  .globl begin_data;                                                           \
  begin_data:

#define RVMODEL_DATA_END                                                       \
  .align 4;                                                                    \
  .globl end_data;                                                             \
  end_data:

#define RVMODEL_HALT                                                           \
  fence;                                                                       \
  li a0, 1;                                                                    \
  li a7, 93;                                                                   \
  li a0, 0;                                                                    \
  ecall

#define RVMODEL_PASS                                                           \
  li a0, 0;                                                                    \
  li a7, 93;                                                                   \
  ecall

#define RVMODEL_FAIL                                                           \
  li a0, 1;                                                                    \
  li a7, 93;                                                                   \
  ecall

#define RVMODEL_IO_ASSERT_GPR_EQ(ScrReg, Reg, Value)                           \
  li  ScrReg, Value;                                                           \
  beq ScrReg, Reg, 1f;                                                         \
  RVMODEL_FAIL;                                                                \
  1:

// C extension ebreak insn tests use IO
#define RVMODEL_IO_WRITE_STR(ScrReg, String) c.nop

#endif /* end of include guard: MODEL_TEST_H */
