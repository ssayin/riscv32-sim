#include <p/riscv_test.h>

RVTEST_RV32M

# Test code region.
RVTEST_CODE_BEGIN   # Start of test code.


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
