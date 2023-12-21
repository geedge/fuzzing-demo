cmake_minimum_required(VERSION 3.0)

project(DPDK-integration)

include(ExternalProject)

# Merge multiple patches into one patch file
set(DPDK_PATCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dpdk)
set(MERGED_PATCH_FILE "${CMAKE_BINARY_DIR}/merged_patches.patch")
set(DPDP_PATCH_FILES ${DPDK_PATCH_DIR}/dpdk-21.11.4-memzone-namesize.patch
                    ${DPDK_PATCH_DIR}/dpdk-21.11.4-fm10k-rxtx-vec.patch
                    ${DPDK_PATCH_DIR}/dpdk-21.11.4-mempool-ops-get-count.patch
                    ${DPDK_PATCH_DIR}/dpdk-21.11.4-bypass-flow-err-func.patch
                    ${DPDK_PATCH_DIR}/dpdk-21.11.4-fix-dup-msl-name-sec-process.patch
                    ${DPDK_PATCH_DIR}/dpdk-21.11.4-fix-af-packet-run-as-slave-of-bond.patch
                    ${DPDK_PATCH_DIR}/dpdk-21.11.4-mp-max-fds.patch
                    ${DPDK_PATCH_DIR}/dpdk-21.11.4-dpdk-devbind-dump.patch
                    ${DPDK_PATCH_DIR}/dpdk-21.11.4-dpdk-hugepages-path.patch
                    ${DPDK_PATCH_DIR}/dpdk-21.11.4-dpdk-pcapng-path.patch
                    ${DPDK_PATCH_DIR}/dpdk-21.11.4-extend-max-virtqueue-pairs.patch)
execute_process(COMMAND cat ${DPDP_PATCH_FILES} OUTPUT_FILE ${MERGED_PATCH_FILE})


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release) # 或者 Debug
endif()

if(NOT RTE_MARCH)
	set(RTE_MARCH "corei7")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DPDK_BUILD_TYPE debug)
else()
    set(DPDK_BUILD_TYPE debugoptimized)
endif()

set(DPDK_MESON_DISABLED_DRIVERS "common/cpt,common/dpaax,common/octeontx,common/octeontx2,common/sfc_efx,common/qat,bus/dpaa,bus/fslmc,bus/ifpga,bus/vmbus,mempool/bucket,mempool/dpaa,mempool/dpaa2,mempool/octeontx,mempool/octeontx2,mempool/stack,raw/ifpga,raw/ioat,raw/ntb,raw/octeontx2_dma,raw/octeontx2_ep,raw/skeleton,compress/zlib,regex/octeontx2,vdpa/ifc,event/skeleton,event/dlb,event/dlb2,event/opdl,sevent/keleton,event/sw,event/dsw,baseband/null,baseband/turbo_sw,baseband/fpga_lte_fec,baseband/fpga_5gnr_fec,baseband/acc100,crypto/bcmfs,crypto/ccp,crypto/nitrox,crypto/null,crypto/openssl,crypto/scheduler,crypto/virtio")

# Although default_library=static has been set, both dynamic and static libraries are compiled. a little strange
add_definitions(-DRTE_MAX_MEMZONE=125600)
add_definitions(-DRTE_LIBRTE_GRAPH_STATS=10)
ExternalProject_Add(dpdk PREFIX dpdk
        URL ${CMAKE_CURRENT_SOURCE_DIR}/dpdk/dpdk-21.11.4.tar.xz
        URL_MD5 b24201da07d3fd87cacc2aa03586aed3
        PATCH_COMMAND
                COMMAND patch -Ns -p1 -i ${MERGED_PATCH_FILE}
        CONFIGURE_COMMAND
                COMMAND meson -Dc_args=-DRTE_ETHDEV_PROFILE_WITH_VTUNE -Ddefault_library=static -Ddisable_drivers=${DPDK_MESON_DISABLED_DRIVERS} -Dprefix=<INSTALL_DIR> -Dmachine=${RTE_MARCH} -Dibverbs_link=dlopen -Dbuildtype=${DPDK_BUILD_TYPE} build
        BUILD_COMMAND  ${CMAKE_COMMAND} -E chdir <SOURCE_DIR>/build ninja 
        INSTALL_COMMAND  ${CMAKE_COMMAND} -E chdir <SOURCE_DIR>/build ninja install 
        BUILD_IN_SOURCE 1)

ExternalProject_Get_Property(dpdk INSTALL_DIR)
file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

add_library(libdpdk STATIC IMPORTED GLOBAL)
add_dependencies(libdpdk dpdk)
set_target_properties(libdpdk PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib64/librte_eal.a
    INTERFACE_LINK_DIRECTORIES ${INSTALL_DIR}/lib64
    INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
    IMPORTED_LINK_INTERFACE_LIBRARIES "numa;rte_ethdev.a;rte_telemetry.a;rte_kvargs.a"
)

get_target_property(library_location libdpdk IMPORTED_LINK_INTERFACE_LIBRARIES)
message(STATUS "Library Location: ${library_location}")


add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} PRIVATE libdpdk)
