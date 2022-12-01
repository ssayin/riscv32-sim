project(riscv32-sim-build2 ASM C)
include(../cmake/riscv.cmake)

set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
set(LINKER_SCRIPT "../external/riscv-test-env/p/link.ld")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nodefaultlibs -fno-exceptions -T ${LINKER_SCRIPT}")


#add_library(serdar STATIC crt/crt.c)
#set_target_properties(serdar PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")

set(
        SOURCES
        test1.S
)

foreach (SRC ${SOURCES})
    get_filename_component(TARGET ${SRC} NAME_WLE)

    add_executable(${TARGET} ${SRC})
    set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")
    target_include_directories(${TARGET} PRIVATE ../external/riscv-test-env/p)

    # target_link_libraries(${TARGET} PRIVATE serdar)
endforeach ()

