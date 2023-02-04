#ifndef COMMON_CONSTS_HPP
#define COMMON_CONSTS_HPP

#include <cstdint>

struct masks {
  static constexpr uint32_t sign_bit   = 0x80000000U;
  static constexpr uint32_t lsb_zero   = 0xFFFFFFFEU;
  static constexpr uint32_t msb_zero   = 0x7FFFFFFFU;
  static constexpr uint32_t type_u_imm = 0xFFFFF000U;

  struct tvec {
    static constexpr uint32_t type      = 0x00000002U;
    static constexpr uint32_t base_addr = 0xFFFFFFFCU;
  };

  static constexpr uint32_t ecall  = 0x73U;
  static constexpr uint32_t ebreak = 0x9002U;
  static constexpr uint32_t mret   = 0x30200073U;
  static constexpr uint32_t sret   = 0x10200073U;
  static constexpr uint32_t wfi    = 0x10500073U;

  enum class opcode : uint8_t {
    branch   = 0b1100011,
    load     = 0b0000011,
    store    = 0b0100011,
    reg_imm  = 0b0010011,
    reg_reg  = 0b0110011,
    misc_mem = 0b0001111,
    sys      = 0b1110011,
    lui      = 0b0110111,
    auipc    = 0b0010111,
    jal      = 0b1101111,
    jalr     = 0b1100111,
  };

  enum class branch : uint8_t {
    beq  = 0b000,
    bne  = 0b001,
    blt  = 0b100,
    bge  = 0b101,
    bltu = 0b110,
    bgeu = 0b111
  };

  enum class load : uint8_t {
    lb  = 0b000,
    lh  = 0b001,
    lw  = 0b010,
    lbu = 0b100,
    lhu = 0b101
  };

  enum class store : uint8_t {
    sb = 0b000,
    sh = 0b001,
    sw = 0b010,
  };

  enum class reg_imm : uint8_t {
    addi      = 0b000,
    slti      = 0b010,
    sltiu     = 0b011,
    xori      = 0b100,
    ori       = 0b110,
    andi      = 0b111,
    slli      = 0b001,
    srli_srai = 0b101,
  };

  enum class reg_reg : uint8_t {
    add_sub_mul  = 0b000,
    sll_mulh     = 0b001,
    slt_mulhsu   = 0b010,
    sltu_mulhu   = 0b011,
    xor_div      = 0b100,
    srl_sra_divu = 0b101,
    or_rem       = 0b110,
    and_remu     = 0b111,
  };

  enum class misc_mem : uint8_t {
    fence  = 0b000,
    fencei = 0b001,
  };

  enum class sys : uint8_t {
    other  = 0b000,
    csrrw  = 0b001,
    csrrs  = 0b010,
    csrrc  = 0b011,
    csrrwi = 0b101,
    csrrsi = 0b110,
    csrrci = 0b111,
  };
};

#endif // COMMON_CONSTS_HPP
