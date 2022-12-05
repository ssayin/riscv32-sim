set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
set(RISCV_TEST_ENV_DIR "${PROJECT_SOURCE_DIR}/external/riscv-test-env")
set(LINKER_SCRIPT "${RISCV_TEST_ENV_DIR}/p/link.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nodefaultlibs -fno-exceptions -T ${LINKER_SCRIPT}")
