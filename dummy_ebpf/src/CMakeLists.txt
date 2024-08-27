find_package(PkgConfig)
pkg_check_modules(LIBBPF REQUIRED libbpf)
pkg_check_modules(LIBXDP REQUIRED libxdp)

find_path(ASM_TYPES_H_PATH NAMES asm/types.h PATHS /usr/include/x86_64-linux-gnu)
if(ASM_TYPES_H_PATH)
    message(STATUS "Found asm/types.h at ${ASM_TYPES_H_PATH}")
    include_directories(${ASM_TYPES_H_PATH})
else()
    message(FATAL_ERROR "asm/types.h not found")
endif()

set(BPF_C_FILE ${CMAKE_CURRENT_SOURCE_DIR}/xdp_prog_kernel.c)
set(BPF_O_FILE ${CMAKE_CURRENT_BINARY_DIR}/xdp_prog_kernel.o)
add_custom_command(OUTPUT ${BPF_O_FILE}
    COMMAND clang -g -O2 -target bpf -D__x86_64__ -I${ASM_TYPES_H_PATH} -c ${BPF_C_FILE} -o ${BPF_O_FILE}
    COMMAND_EXPAND_LISTS
    VERBATIM
    DEPENDS ${BPF_C_FILE}
    COMMENT "[clang] Building BPF file: ${BPF_C_FILE}")

add_custom_target(generate_bpf_obj ALL
    DEPENDS ${BPF_O_FILE}
)

set(SOURCE_FILES main.c config.c device.c tap.c xdp_prog_user.c)
add_executable(main ${SOURCE_FILES})
target_include_directories(main PRIVATE ${NL3_INCLUDE_DIRS})
target_link_libraries(main PRIVATE ${NL3_LIBRARIES} ${LIBBPF_LIBRARIES} ${LIBXDP_LIBRARIES})