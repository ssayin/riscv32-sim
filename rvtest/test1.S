#include "riscv_test.h"

RVTEST_RV32M        # Define TVM used by program.

# Test code region.
RVTEST_CODE_BEGIN   # Start of test code.
        auipc	sp, 0x2
        lw      x2, -392(sp)
        addi    x2, x0, 1     # Should be 42 into $2.
        auipc	tp, 0x2
        sw      x2, -388(tp)    # Store result into memory overwriting 1s.
        li      x3, 42        # Desired result.
        bne     x2, x3, fail  # Fail out if doesn't match.
        RVTEST_PASS           # Signal success.
fail:
        RVTEST_FAIL
RVTEST_CODE_END     # End of test code.

# Input data section.
# This section is optional, and this data is NOT saved in the output.
.data
        .align 3
testdata:
        .dword 41

# Output data section.
RVTEST_DATA_BEGIN   # Start of test output data region.
        .align 3
result:
        .dword -1
RVTEST_DATA_END     # End of test output data region.