/*
 * RISC-V Disassembler
 *
 * Copyright (c) 2016-2017 Michael Clark <michaeljclark@mac.com>
 * Copyright (c) 2017-2018 SiFive, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef RISCV_DISASSEMBLER_H
#define RISCV_DISASSEMBLER_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

/* types */

typedef uint64_t rv_inst;
typedef uint16_t rv_opcode;

/* enums */

typedef enum {
  rv32,
  rv64,
  rv128
} rv_isa;

typedef enum {
  rv_rm_rne = 0,
  rv_rm_rtz = 1,
  rv_rm_rdn = 2,
  rv_rm_rup = 3,
  rv_rm_rmm = 4,
  rv_rm_dyn = 7,
} rv_rm;

typedef enum {
  rv_fence_i = 8,
  rv_fence_o = 4,
  rv_fence_r = 2,
  rv_fence_w = 1,
} rv_fence;

typedef enum {
  rv_ireg_zero,
  rv_ireg_ra,
  rv_ireg_sp,
  rv_ireg_gp,
  rv_ireg_tp,
  rv_ireg_t0,
  rv_ireg_t1,
  rv_ireg_t2,
  rv_ireg_s0,
  rv_ireg_s1,
  rv_ireg_a0,
  rv_ireg_a1,
  rv_ireg_a2,
  rv_ireg_a3,
  rv_ireg_a4,
  rv_ireg_a5,
  rv_ireg_a6,
  rv_ireg_a7,
  rv_ireg_s2,
  rv_ireg_s3,
  rv_ireg_s4,
  rv_ireg_s5,
  rv_ireg_s6,
  rv_ireg_s7,
  rv_ireg_s8,
  rv_ireg_s9,
  rv_ireg_s10,
  rv_ireg_s11,
  rv_ireg_t3,
  rv_ireg_t4,
  rv_ireg_t5,
  rv_ireg_t6,
} rv_ireg;

typedef enum {
  rvc_end,
  rvc_rd_eq_ra,
  rvc_rd_eq_x0,
  rvc_rs1_eq_x0,
  rvc_rs2_eq_x0,
  rvc_rs2_eq_rs1,
  rvc_rs1_eq_ra,
  rvc_imm_eq_zero,
  rvc_imm_eq_n1,
  rvc_imm_eq_p1,
  rvc_csr_eq_0x001,
  rvc_csr_eq_0x002,
  rvc_csr_eq_0x003,
  rvc_csr_eq_0xc00,
  rvc_csr_eq_0xc01,
  rvc_csr_eq_0xc02,
  rvc_csr_eq_0xc80,
  rvc_csr_eq_0xc81,
  rvc_csr_eq_0xc82,
} rvc_constraint;

typedef enum {
  rv_codec_illegal,
  rv_codec_none,
  rv_codec_u,
  rv_codec_uj,
  rv_codec_i,
  rv_codec_i_sh5,
  rv_codec_i_sh6,
  rv_codec_i_sh7,
  rv_codec_i_csr,
  rv_codec_s,
  rv_codec_sb,
  rv_codec_r,
  rv_codec_r_m,
  rv_codec_r4_m,
  rv_codec_r_a,
  rv_codec_r_l,
  rv_codec_r_f,
  rv_codec_cb,
  rv_codec_cb_imm,
  rv_codec_cb_sh5,
  rv_codec_cb_sh6,
  rv_codec_ci,
  rv_codec_ci_sh5,
  rv_codec_ci_sh6,
  rv_codec_ci_16sp,
  rv_codec_ci_lwsp,
  rv_codec_ci_ldsp,
  rv_codec_ci_lqsp,
  rv_codec_ci_li,
  rv_codec_ci_lui,
  rv_codec_ci_none,
  rv_codec_ciw_4spn,
  rv_codec_cj,
  rv_codec_cj_jal,
  rv_codec_cl_lw,
  rv_codec_cl_ld,
  rv_codec_cl_lq,
  rv_codec_cr,
  rv_codec_cr_mv,
  rv_codec_cr_jalr,
  rv_codec_cr_jr,
  rv_codec_cs,
  rv_codec_cs_sw,
  rv_codec_cs_sd,
  rv_codec_cs_sq,
  rv_codec_css_swsp,
  rv_codec_css_sdsp,
  rv_codec_css_sqsp,
} rv_codec;

typedef enum {
  rv_op_illegal,
  rv_op_lui,
  rv_op_auipc,
  rv_op_jal,
  rv_op_jalr,
  rv_op_beq,
  rv_op_bne,
  rv_op_blt,
  rv_op_bge,
  rv_op_bltu,
  rv_op_bgeu,
  rv_op_lb,
  rv_op_lh,
  rv_op_lw,
  rv_op_lbu,
  rv_op_lhu,
  rv_op_sb,
  rv_op_sh,
  rv_op_sw,
  rv_op_addi,
  rv_op_slti,
  rv_op_sltiu,
  rv_op_xori,
  rv_op_ori,
  rv_op_andi,
  rv_op_slli,
  rv_op_srli,
  rv_op_srai,
  rv_op_add,
  rv_op_sub,
  rv_op_sll,
  rv_op_slt,
  rv_op_sltu,
  rv_op_xor,
  rv_op_srl,
  rv_op_sra,
  rv_op_or,
  rv_op_and,
  rv_op_fence,
  rv_op_fence_i,
  rv_op_lwu,
  rv_op_ld,
  rv_op_sd,
  rv_op_addiw,
  rv_op_slliw,
  rv_op_srliw,
  rv_op_sraiw,
  rv_op_addw,
  rv_op_subw,
  rv_op_sllw,
  rv_op_srlw,
  rv_op_sraw,
  rv_op_ldu,
  rv_op_lq,
  rv_op_sq,
  rv_op_addid,
  rv_op_sllid,
  rv_op_srlid,
  rv_op_sraid,
  rv_op_addd,
  rv_op_subd,
  rv_op_slld,
  rv_op_srld,
  rv_op_srad,
  rv_op_mul,
  rv_op_mulh,
  rv_op_mulhsu,
  rv_op_mulhu,
  rv_op_div,
  rv_op_divu,
  rv_op_rem,
  rv_op_remu,
  rv_op_mulw,
  rv_op_divw,
  rv_op_divuw,
  rv_op_remw,
  rv_op_remuw,
  rv_op_muld,
  rv_op_divd,
  rv_op_divud,
  rv_op_remd,
  rv_op_remud,
  rv_op_lr_w,
  rv_op_sc_w,
  rv_op_amoswap_w,
  rv_op_amoadd_w,
  rv_op_amoxor_w,
  rv_op_amoor_w,
  rv_op_amoand_w,
  rv_op_amomin_w,
  rv_op_amomax_w,
  rv_op_amominu_w,
  rv_op_amomaxu_w,
  rv_op_lr_d,
  rv_op_sc_d,
  rv_op_amoswap_d,
  rv_op_amoadd_d,
  rv_op_amoxor_d,
  rv_op_amoor_d,
  rv_op_amoand_d,
  rv_op_amomin_d,
  rv_op_amomax_d,
  rv_op_amominu_d,
  rv_op_amomaxu_d,
  rv_op_lr_q,
  rv_op_sc_q,
  rv_op_amoswap_q,
  rv_op_amoadd_q,
  rv_op_amoxor_q,
  rv_op_amoor_q,
  rv_op_amoand_q,
  rv_op_amomin_q,
  rv_op_amomax_q,
  rv_op_amominu_q,
  rv_op_amomaxu_q,
  rv_op_ecall,
  rv_op_ebreak,
  rv_op_uret,
  rv_op_sret,
  rv_op_hret,
  rv_op_mret,
  rv_op_dret,
  rv_op_sfence_vm,
  rv_op_sfence_vma,
  rv_op_wfi,
  rv_op_csrrw,
  rv_op_csrrs,
  rv_op_csrrc,
  rv_op_csrrwi,
  rv_op_csrrsi,
  rv_op_csrrci,
  rv_op_flw,
  rv_op_fsw,
  rv_op_fmadd_s,
  rv_op_fmsub_s,
  rv_op_fnmsub_s,
  rv_op_fnmadd_s,
  rv_op_fadd_s,
  rv_op_fsub_s,
  rv_op_fmul_s,
  rv_op_fdiv_s,
  rv_op_fsgnj_s,
  rv_op_fsgnjn_s,
  rv_op_fsgnjx_s,
  rv_op_fmin_s,
  rv_op_fmax_s,
  rv_op_fsqrt_s,
  rv_op_fle_s,
  rv_op_flt_s,
  rv_op_feq_s,
  rv_op_fcvt_w_s,
  rv_op_fcvt_wu_s,
  rv_op_fcvt_s_w,
  rv_op_fcvt_s_wu,
  rv_op_fmv_x_s,
  rv_op_fclass_s,
  rv_op_fmv_s_x,
  rv_op_fcvt_l_s,
  rv_op_fcvt_lu_s,
  rv_op_fcvt_s_l,
  rv_op_fcvt_s_lu,
  rv_op_fld,
  rv_op_fsd,
  rv_op_fmadd_d,
  rv_op_fmsub_d,
  rv_op_fnmsub_d,
  rv_op_fnmadd_d,
  rv_op_fadd_d,
  rv_op_fsub_d,
  rv_op_fmul_d,
  rv_op_fdiv_d,
  rv_op_fsgnj_d,
  rv_op_fsgnjn_d,
  rv_op_fsgnjx_d,
  rv_op_fmin_d,
  rv_op_fmax_d,
  rv_op_fcvt_s_d,
  rv_op_fcvt_d_s,
  rv_op_fsqrt_d,
  rv_op_fle_d,
  rv_op_flt_d,
  rv_op_feq_d,
  rv_op_fcvt_w_d,
  rv_op_fcvt_wu_d,
  rv_op_fcvt_d_w,
  rv_op_fcvt_d_wu,
  rv_op_fclass_d,
  rv_op_fcvt_l_d,
  rv_op_fcvt_lu_d,
  rv_op_fmv_x_d,
  rv_op_fcvt_d_l,
  rv_op_fcvt_d_lu,
  rv_op_fmv_d_x,
  rv_op_flq,
  rv_op_fsq,
  rv_op_fmadd_q,
  rv_op_fmsub_q,
  rv_op_fnmsub_q,
  rv_op_fnmadd_q,
  rv_op_fadd_q,
  rv_op_fsub_q,
  rv_op_fmul_q,
  rv_op_fdiv_q,
  rv_op_fsgnj_q,
  rv_op_fsgnjn_q,
  rv_op_fsgnjx_q,
  rv_op_fmin_q,
  rv_op_fmax_q,
  rv_op_fcvt_s_q,
  rv_op_fcvt_q_s,
  rv_op_fcvt_d_q,
  rv_op_fcvt_q_d,
  rv_op_fsqrt_q,
  rv_op_fle_q,
  rv_op_flt_q,
  rv_op_feq_q,
  rv_op_fcvt_w_q,
  rv_op_fcvt_wu_q,
  rv_op_fcvt_q_w,
  rv_op_fcvt_q_wu,
  rv_op_fclass_q,
  rv_op_fcvt_l_q,
  rv_op_fcvt_lu_q,
  rv_op_fcvt_q_l,
  rv_op_fcvt_q_lu,
  rv_op_fmv_x_q,
  rv_op_fmv_q_x,
  rv_op_c_addi4spn,
  rv_op_c_fld,
  rv_op_c_lw,
  rv_op_c_flw,
  rv_op_c_fsd,
  rv_op_c_sw,
  rv_op_c_fsw,
  rv_op_c_nop,
  rv_op_c_addi,
  rv_op_c_jal,
  rv_op_c_li,
  rv_op_c_addi16sp,
  rv_op_c_lui,
  rv_op_c_srli,
  rv_op_c_srai,
  rv_op_c_andi,
  rv_op_c_sub,
  rv_op_c_xor,
  rv_op_c_or,
  rv_op_c_and,
  rv_op_c_subw,
  rv_op_c_addw,
  rv_op_c_j,
  rv_op_c_beqz,
  rv_op_c_bnez,
  rv_op_c_slli,
  rv_op_c_fldsp,
  rv_op_c_lwsp,
  rv_op_c_flwsp,
  rv_op_c_jr,
  rv_op_c_mv,
  rv_op_c_ebreak,
  rv_op_c_jalr,
  rv_op_c_add,
  rv_op_c_fsdsp,
  rv_op_c_swsp,
  rv_op_c_fswsp,
  rv_op_c_ld,
  rv_op_c_sd,
  rv_op_c_addiw,
  rv_op_c_ldsp,
  rv_op_c_sdsp,
  rv_op_c_lq,
  rv_op_c_sq,
  rv_op_c_lqsp,
  rv_op_c_sqsp,
  rv_op_nop,
  rv_op_mv,
  rv_op_not,
  rv_op_neg,
  rv_op_negw,
  rv_op_sext_w,
  rv_op_seqz,
  rv_op_snez,
  rv_op_sltz,
  rv_op_sgtz,
  rv_op_fmv_s,
  rv_op_fabs_s,
  rv_op_fneg_s,
  rv_op_fmv_d,
  rv_op_fabs_d,
  rv_op_fneg_d,
  rv_op_fmv_q,
  rv_op_fabs_q,
  rv_op_fneg_q,
  rv_op_beqz,
  rv_op_bnez,
  rv_op_blez,
  rv_op_bgez,
  rv_op_bltz,
  rv_op_bgtz,
  rv_op_ble,
  rv_op_bleu,
  rv_op_bgt,
  rv_op_bgtu,
  rv_op_j,
  rv_op_ret,
  rv_op_jr,
  rv_op_rdcycle,
  rv_op_rdtime,
  rv_op_rdinstret,
  rv_op_rdcycleh,
  rv_op_rdtimeh,
  rv_op_rdinstreth,
  rv_op_frcsr,
  rv_op_frrm,
  rv_op_frflags,
  rv_op_fscsr,
  rv_op_fsrm,
  rv_op_fsflags,
  rv_op_fsrmi,
  rv_op_fsflagsi,
} rv_op;

/* structures */

typedef struct {
  uint64_t pc;
  uint64_t inst;
  int32_t  imm;
  uint16_t op;
  uint8_t  codec;
  uint8_t  rd;
  uint8_t  rs1;
  uint8_t  rs2;
  uint8_t  rs3;
  uint8_t  rm;
  uint8_t  pred;
  uint8_t  succ;
  uint8_t  aq;
  uint8_t  rl;
} rv_decode;

/* functions */

size_t inst_length(rv_inst inst);
void   inst_fetch(const uint8_t *data, rv_inst *instp, size_t *length);
void   disasm_inst(char *buf, size_t buflen, rv_isa isa, uint64_t pc,
                   rv_inst inst);

#endif
