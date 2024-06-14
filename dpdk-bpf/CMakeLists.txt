cmake_minimum_required(VERSION 3.11)

project(dpdk-bpf)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=corei7")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=corei7")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBDPDK REQUIRED libdpdk)
include_directories(${LIBDPDK_STATIC_INCLUDE_DIRS})
message(STATUS "LIBDPDK_LIBDIR: ${LIBDPDK_LIBDIR}")

add_custom_target(compress_dpdk_libraries
    COMMAND rm -rf libdpdks.a
    COMMAND sh -c "ar crsT libdpdks.a ./librte*.a"
    WORKING_DIRECTORY ${LIBDPDK_LIBDIR}
    COMMENT "Compressing static libraries"
)

find_library(M_LIB m)
add_library(libdpdk STATIC IMPORTED GLOBAL)
add_dependencies(libdpdk compress_dpdk_libraries)
set_target_properties(libdpdk PROPERTIES
    IMPORTED_LOCATION ${M_LIB}
    INTERFACE_LINK_LIBRARIES "-Wl,--whole-archive;${LIBDPDK_LIBDIR}/libdpdks.a;-Wl,--no-whole-archive;rt;m;numa;fdt;atomic;mlx5;ibverbs;crypto;z;jansson;pcap;elf;pthread;dl"
)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} PRIVATE libdpdk)
