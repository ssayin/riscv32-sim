project(riscv32-sim-build C)
include(../cmake/riscv.cmake)

# set(CMAKE_C_FLAGS "-Os -g -Wall -ffunction-sections")
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/serdar.ld")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nodefaultlibs -fno-exceptions -T ${LINKER_SCRIPT}")


add_library(serdar STATIC crt/crt.c)
set_target_properties(serdar PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")

set(
        SOURCES
        example/test.c
        example/test_csr.c
        example/fib.c
)

foreach (SRC ${SOURCES})
    get_filename_component(TARGET ${SRC} NAME_WLE)

    add_executable(${TARGET} ${SRC})
    set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")
    target_link_libraries(${TARGET} PRIVATE serdar)
endforeach ()

