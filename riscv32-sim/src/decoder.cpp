#include "decoder.hpp"
#include "rv32_isn.hpp"

static op decode_load(uint32_t word);
static op decode_store(uint32_t word);
static op decode_branch(uint32_t word);
static op decode_reg_imm(uint32_t word);
static op decode_alu(uint32_t word);
static op decode_sys(uint32_t word);
static op decode_fence(uint32_t word);

constexpr op make_NOP() {
  return {true, 0, 0, 0, alu_type::_add, pipeline_target::alu, 0};
}

constexpr op make_illegal() { return {{}, {}, {}, {}, {}, {}, {}}; }

op decode(uint32_t word) {
  switch (word) {
    // ecall
  case 0x73:
    return {{}, {}, {}, {}, {}, pipeline_target::ecall, {}};

    // ebreak
  case 0x9002:
    return {{}, {}, {}, {}, {}, pipeline_target::ebreak, {}};

    // mret
  case 0x30200073:
    return {false, 0, 0, 0, {}, pipeline_target::mret, 0};

    // sret
  case 0x10200073:
    return make_NOP();

    // wfi
  case 0x10500073:
    return make_NOP();
  }

  switch (static_cast<opcode>(OPCODE(word))) {
    using enum opcode;
  case auipc: {
    rv32_auipc isn{word};
    return {true,   isn.rd, 0, 0, alu_type::_auipc, pipeline_target::alu,
            isn.imm};
  }
  case lui: {
    rv32_lui isn{word};
    return {true, isn.rd, 0, 0, alu_type::_add, pipeline_target::alu, isn.imm};
  }
  case jal: {
    rv32_jal isn{word};
    return {true, isn.rd, 0, 0, alu_type::_jal, pipeline_target::alu, isn.imm};
  }
  case jalr: {
    rv32_jalr isn{word};
    return {true,   isn.rd, isn.rs, 0, alu_type::_jalr, pipeline_target::alu,
            isn.imm};
  }
  case load:
    return decode_load(word);
  case store:
    return decode_store(word);
  case branch:
    return decode_branch(word);
  case reg_imm:
    return decode_reg_imm(word);
  case reg_reg:
    return decode_alu(word);
  case sys:
    return decode_sys(word);
  case misc_mem:
    return decode_fence(word);
  default:
    return make_illegal();
  }
}

#define RV32_LOAD(name)                                                        \
  case load::name: {                                                           \
    rv32_##name isn{word};                                                     \
    return {true,   isn.rd, isn.rs, 0, mem_type::name, pipeline_target::mem,   \
            isn.imm};                                                          \
  }

static op decode_load(uint32_t word) {
  switch (static_cast<load>(FUNCT3(word))) {
    RV32_LOAD(lb)
    RV32_LOAD(lh)
    RV32_LOAD(lw)
    RV32_LOAD(lbu)
    RV32_LOAD(lhu)
  default:
    return make_illegal();
  }
}

#undef RV32_LOAD

#define RV32_STORE(name)                                                       \
  case store::name: {                                                          \
    rv32_##name isn{word};                                                     \
    return {true,   0, isn.rs1, isn.rs2, mem_type::name, pipeline_target::mem, \
            isn.imm};                                                          \
  }

op decode_store(uint32_t word) {
  switch (static_cast<store>(FUNCT3(word))) {
    RV32_STORE(sb)
    RV32_STORE(sh)
    RV32_STORE(sw)
  default:
    return make_illegal();
  }
}

#undef RV32_STORE

#define RV32_REG_REG(name, funct7)                                             \
  case funct7: {                                                               \
    rv32_##name isn{word};                                                     \
    return {false,                                                             \
            isn.rd,                                                            \
            isn.rs1,                                                           \
            isn.rs2,                                                           \
            alu_type::_##name,                                                 \
            pipeline_target::alu,                                              \
            0};                                                                \
  }

static op decode_alu_and_remu(uint32_t word) {
  switch (FUNCT7(word)) {
    RV32_REG_REG(and, 0x0)
    RV32_REG_REG(remu, 0x1)
  default:
    return make_illegal();
  }
}

static op decode_alu_or_rem(uint32_t word) {
  switch (FUNCT7(word)) {
    RV32_REG_REG(or, 0x0)
    RV32_REG_REG(rem, 0x1)
  default:
    return make_illegal();
  }
}

static op decode_alu_xor_div(uint32_t word) {
  switch (FUNCT7(word)) {
    RV32_REG_REG(xor, 0x0)
    RV32_REG_REG(div, 0x1)
  default:
    return make_illegal();
  }
}

static op decode_alu_add_sub_mul(uint32_t word) {
  switch (FUNCT7(word)) {
    RV32_REG_REG(add, 0x0)
    RV32_REG_REG(mul, 0x1)
    RV32_REG_REG(sub, 0x20)
  default:
    return make_illegal();
  }
}

static op decode_alu_sll_mulh(uint32_t word) {
  switch (FUNCT7(word)) {
    RV32_REG_REG(sll, 0x0)
    RV32_REG_REG(mulh, 0x1)
  default:
    return make_illegal();
  }
}

static op decode_alu_srl_sra_divu(uint32_t word) {
  switch (FUNCT7(word)) {
    RV32_REG_REG(srl, 0x0)
    RV32_REG_REG(divu, 0x1)
    RV32_REG_REG(sra, 0x20)
  default:
    return make_illegal();
  }
}

static op decode_alu_slt_mulhsu(uint32_t word) {
  switch (FUNCT7(word)) {
    RV32_REG_REG(slt, 0x0)
    RV32_REG_REG(mulhsu, 0x1)
  default:
    return make_illegal();
  }
}

static op decode_alu_sltu_mulhu(uint32_t word) {
  switch (FUNCT7(word)) {
    RV32_REG_REG(sltu, 0x0)
    RV32_REG_REG(mulhu, 0x1)
  default:
    return make_illegal();
  }
}

#undef RV32_REG_REG

static op decode_alu(uint32_t word) {
  switch (static_cast<reg_reg>(FUNCT3(word))) {
    using enum reg_reg;
  case and_remu:
    return decode_alu_and_remu(word);
  case or_rem:
    return decode_alu_or_rem(word);
  case xor_div:
    return decode_alu_xor_div(word);
  case add_sub_mul:
    return decode_alu_add_sub_mul(word);
  case sll_mulh:
    return decode_alu_sll_mulh(word);
  case srl_sra_divu:
    return decode_alu_srl_sra_divu(word);
  case slt_mulhsu:
    return decode_alu_slt_mulhsu(word);
  case sltu_mulhu:
    return decode_alu_sltu_mulhu(word);
  default:
    return make_illegal();
  }
}

#define RV32_REG_IMM(name)                                                     \
  case name##i: {                                                              \
    rv32_##name##i isn{word};                                                  \
    return {                                                                   \
        true,   isn.rd, isn.rs, 0, alu_type::_##name, pipeline_target::alu,    \
        isn.imm};                                                              \
  }

static op decode_reg_imm(uint32_t word) {
  switch (static_cast<reg_imm>(FUNCT3(word))) {
    using enum reg_imm;
    RV32_REG_IMM(add)
    RV32_REG_IMM(slt)
    RV32_REG_IMM(xor)
    RV32_REG_IMM(or)
    RV32_REG_IMM(and)
    RV32_REG_IMM(sll)

  case srli_srai: {
    constexpr int srli = 0x0;
    constexpr int srai = 0x20;
    switch (FUNCT7(word)) {
      RV32_REG_IMM(srl)
      RV32_REG_IMM(sra)
    default:
      return make_illegal();
    }
  }

  case sltiu: {
    rv32_sltiu isn{word};
    return {true,   isn.rd, isn.rs, 0, alu_type::_sltu, pipeline_target::alu,
            isn.imm};
  }
  default:
    return make_illegal();
  }
}

#undef RV32_REG_IMM

#define RV32_BRANCH(name)                                                      \
  case branch::name: {                                                         \
    rv32_##name isn{word};                                                     \
    return {true,                                                              \
            0,                                                                 \
            isn.rs1,                                                           \
            isn.rs2,                                                           \
            branch_type::name,                                                 \
            pipeline_target::branch,                                           \
            isn.imm};                                                          \
  }

static op decode_branch(uint32_t word) {
  switch (static_cast<branch>(FUNCT3(word))) {
    RV32_BRANCH(beq)
    RV32_BRANCH(bne)
    RV32_BRANCH(blt)
    RV32_BRANCH(bltu)
    RV32_BRANCH(bge)
    RV32_BRANCH(bgeu)
  default:
    return make_illegal();
  }
}

#undef RV32_BRANCH

static op decode_fence(uint32_t word) { return make_NOP(); }

#define RV32_CSR(name)                                                         \
  case sys::name: {                                                            \
    rv32_##name isn{word};                                                     \
    return {true,   isn.rd, isn.rs, 0, csr_type::name, pipeline_target::csr,   \
            isn.csr};                                                          \
  }

static op decode_sys(uint32_t word) {
  switch (static_cast<sys>(FUNCT3(word))) {
    RV32_CSR(csrrw)
    RV32_CSR(csrrs)
    RV32_CSR(csrrc)
    RV32_CSR(csrrwi)
    RV32_CSR(csrrsi)
    RV32_CSR(csrrci)
  default:
    return make_illegal();
  }
}

#undef RV32_CSR
