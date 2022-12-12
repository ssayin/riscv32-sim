#include <p/riscv_test.h>

RVTEST_RV32M

# Test code region.
RVTEST_CODE_BEGIN   # Start of test code.
        nop
        li      x2, 41
        addi    x2, x2, 1     # Should be 42 into $2.
        sw      x2, result, x17    # Store result into memory overwriting 1s.
        li      x3, 42        # Desired result.
        lw      x29, result
        bne     x2, x3, fail  # Fail out if doesn't match.
        RVTEST_PASS           # Signal success.
fail:
        RVTEST_FAIL
RVTEST_CODE_END     # End of test code.

# Output data section.
RVTEST_DATA_BEGIN   # Start of test output data region.
        .align 3
result:
        .word -1
RVTEST_DATA_END     # End of test output data region.
