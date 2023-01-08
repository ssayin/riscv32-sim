#include "decoder/decoder.hpp"
#include "instr/rv32_isn.hpp"
#include <fmt/os.h>
#include <fmt/ostream.h>

static op decode_load(uint32_t word);
static op decode_store(uint32_t word);
static op decode_branch(uint32_t word);
static op decode_reg_imm(uint32_t word);
static op decode_alu(uint32_t word);
static op decode_sys(uint32_t word);
static op decode_fence(uint32_t word);

constexpr op make_nop() { return op{0, alu::_add, target::alu, 0, 0, 0, true}; }

constexpr op make_illegal() {
  return op{0, {}, target::illegal, 0, 0, 0, false};
}

op decode(uint32_t word) {
  switch (word) {
  case ecall:
    return op{0, {}, target::ecall, 0, 0, 0, false};
  case ebreak:
    return op{0, {}, target::ebreak, 0, 0, 0, false};
  case mret:
    return op{0, {}, target::mret, 0, 0, 0, false};
  case sret:
    return make_nop();

    // wfi
  case 0x10500073:
    return make_nop();
  }

  switch (static_cast<opcode>(off::opc(word))) {
    using enum opcode;
  case auipc: {
    rv32_auipc isn{word};

    return op{isn.imm, alu::_auipc, target::alu, isn.rd, 0, 0, true};
  }
  case lui: {
    rv32_lui isn{word};

    return op{isn.imm, alu::_add, target::alu, isn.rd, 0, 0, true};
  }
  case jal: {
    rv32_jal isn{word};

    return op{isn.imm, alu::_jal, target::alu, isn.rd, 0, 0, true};
  }
  case jalr: {
    rv32_jalr isn{word};

    return op{isn.imm, alu::_jalr, target::alu, isn.rd, isn.rs, 0, true};
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
                                                                               \
    return op{isn.imm, load::name, target::load, isn.rd, isn.rs, 0, true};     \
  }

static op decode_load(uint32_t word) {
  switch (static_cast<load>(off::funct3(word))) {
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
                                                                               \
    return op{isn.imm, store::name, target::store, 0, isn.rs1, isn.rs2, true}; \
  }

op decode_store(uint32_t word) {
  switch (static_cast<store>(off::funct3(word))) {
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
                                                                               \
    return op{0, alu::_##name, target::alu, isn.rd, isn.rs1, isn.rs2, false};  \
  }

static op decode_alu_and_remu(uint32_t word) {
  switch (off::funct7(word)) {
    RV32_REG_REG(and, 0x0)
    RV32_REG_REG(remu, 0x1)
  default:
    return make_illegal();
  }
}

static op decode_alu_or_rem(uint32_t word) {
  switch (off::funct7(word)) {
    RV32_REG_REG(or, 0x0)
    RV32_REG_REG(rem, 0x1)
  default:
    return make_illegal();
  }
}

static op decode_alu_xor_div(uint32_t word) {
  switch (off::funct7(word)) {
    RV32_REG_REG(xor, 0x0)
    RV32_REG_REG(div, 0x1)
  default:
    return make_illegal();
  }
}

static op decode_alu_add_sub_mul(uint32_t word) {
  switch (off::funct7(word)) {
    RV32_REG_REG(add, 0x0)
    RV32_REG_REG(mul, 0x1)
    RV32_REG_REG(sub, 0x20)
  default:
    return make_illegal();
  }
}

static op decode_alu_sll_mulh(uint32_t word) {
  switch (off::funct7(word)) {
    RV32_REG_REG(sll, 0x0)
    RV32_REG_REG(mulh, 0x1)
  default:
    return make_illegal();
  }
}

static op decode_alu_srl_sra_divu(uint32_t word) {
  switch (off::funct7(word)) {
    RV32_REG_REG(srl, 0x0)
    RV32_REG_REG(divu, 0x1)
    RV32_REG_REG(sra, 0x20)
  default:
    return make_illegal();
  }
}

static op decode_alu_slt_mulhsu(uint32_t word) {
  switch (off::funct7(word)) {
    RV32_REG_REG(slt, 0x0)
    RV32_REG_REG(mulhsu, 0x1)
  default:
    return make_illegal();
  }
}

static op decode_alu_sltu_mulhu(uint32_t word) {
  switch (off::funct7(word)) {
    RV32_REG_REG(sltu, 0x0)
    RV32_REG_REG(mulhu, 0x1)
  default:
    return make_illegal();
  }
}

#undef RV32_REG_REG

static op decode_alu(uint32_t word) {
  switch (static_cast<reg_reg>(off::funct3(word))) {
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
                                                                               \
    return op{isn.imm, alu::_##name, target::alu, isn.rd, isn.rs, 0, true};    \
  }

static op decode_reg_imm(uint32_t word) {
  switch (static_cast<reg_imm>(off::funct3(word))) {
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
    switch (off::funct7(word)) {
      RV32_REG_IMM(srl)
      RV32_REG_IMM(sra)
    default:
      return make_illegal();
    }
  }

  case sltiu: {
    rv32_sltiu isn{word};

    return op{isn.imm, alu::_sltu, target::alu, isn.rd, isn.rs, 0, true};
  }
  default:
    return make_illegal();
  }
}

#undef RV32_REG_IMM

#define RV32_BRANCH(name)                                                      \
  case branch::name: {                                                         \
    rv32_##name isn{word};                                                     \
                                                                               \
    return op{                                                                 \
        isn.imm, branch::name, target::branch, 0, isn.rs1, isn.rs2, true};     \
  }

static op decode_branch(uint32_t word) {
  switch (static_cast<branch>(off::funct3(word))) {
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

static op decode_fence(uint32_t word) { return make_nop(); }

#define RV32_CSR(name)                                                         \
  case sys::name: {                                                            \
    rv32_##name isn{word};                                                     \
                                                                               \
    return op{isn.csr, sys::name, target::csr, isn.rd, isn.rs, 0, true};       \
  }

static op decode_sys(uint32_t word) {
  switch (static_cast<sys>(off::funct3(word))) {
    RV32_CSR(csrrw)
    RV32_CSR(csrrs)
    RV32_CSR(csrrc)
    RV32_CSR(csrrwi)
    RV32_CSR(csrrsi)
    RV32_CSR(csrrci)
  default:
    // those are covered in decode(...), this is an unimplemented system instr
    return make_illegal();
  }
}

#undef RV32_CSR
