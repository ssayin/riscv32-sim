include(riscv)
include(riscv_test_env)

set(
        SOURCES
        test1.S
)

foreach (SRC ${SOURCES})
    get_filename_component(TARGET ${SRC} NAME_WLE)
    add_executable(${TARGET} ${SRC})
    set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")
    target_include_directories(${TARGET} PRIVATE "${RISCV_TEST_ENV_DIR}")
endforeach ()

