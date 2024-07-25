# This file configures the build for the microshell submodule, which is included
# as source for the CLI which will be built as a static library to be used by
# the main application.

# pull in microshell setup
add_subdirectory(${PROJECT_SOURCE_DIR}/microshell/src microshell)

# run microshell build setup script
build_ush(microshell)

# defines to use in microshell build
target_compile_definitions(
    microshell
    PUBLIC
    PICO
)

# create cli library to use at top level (linked against microshell)
add_library(cli STATIC
    ${CMAKE_CURRENT_LIST_DIR}/shell.c
    ${CMAKE_CURRENT_LIST_DIR}/shell_cmd.c
    ${CMAKE_CURRENT_LIST_DIR}/cli_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/node_root.c
    ${CMAKE_CURRENT_LIST_DIR}/node_dev.c
    ${CMAKE_CURRENT_LIST_DIR}/node_bin.c
    ${CMAKE_CURRENT_LIST_DIR}/node_proc.c
    ${CMAKE_CURRENT_LIST_DIR}/node_mnt.c
    ${CMAKE_CURRENT_LIST_DIR}/node_etc.c
    ${CMAKE_CURRENT_LIST_DIR}/node_lib.c
    # sekim 20240614 CMakeList Changed : add source files
    ${CMAKE_CURRENT_LIST_DIR}/w5500_func.c
    ${CMAKE_CURRENT_LIST_DIR}/w5500_pico_func.c
    ${CMAKE_CURRENT_LIST_DIR}/w5500_pico_peri_func.c
    ${CMAKE_CURRENT_LIST_DIR}/w5500_misc.c
    ${CMAKE_CURRENT_LIST_DIR}/w5500_polltask.c
    ${CMAKE_CURRENT_LIST_DIR}/w5500_registerview.c
    ${PROJECT_SOURCE_DIR}/libraries/pico-extras/src/rp2_common/pico_rand/rand.c
)
target_link_libraries(cli PUBLIC
    microshell
    pico_stdlib
    FreeRTOS-Kernel
    cmake_git_version_tracking
)
target_include_directories(cli PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/rtos
    ${PROJECT_SOURCE_DIR}/hardware/rp2040
    ${PROJECT_SOURCE_DIR}/driver_lib
    ${PROJECT_SOURCE_DIR}/services
    ${PROJECT_SOURCE_DIR}/littlefs/littlefs
    ${PROJECT_SOURCE_DIR}/git_version
    # sekim 20240614 CMakeList Changed : add include directory 
    ${PROJECT_SOURCE_DIR}/libraries/ioLibrary_Driver/Ethernet
    ${PROJECT_SOURCE_DIR}/libraries/ioLibrary_Driver/Ethernet/W5500
    ${PROJECT_SOURCE_DIR}/port/ioLibrary_Driver/inc
    ${PROJECT_SOURCE_DIR}/port/lwip
    ${PROJECT_SOURCE_DIR}/port/pio/include
    ${PROJECT_SOURCE_DIR}/port
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_lwip/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_rand/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_spi/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_dma/include
)

target_compile_options(
    cli
    PRIVATE
    # sekim 20240614 CMakeList Changed : waring is not error 
    #-Werror -g -O0
    -g -O3
)
# TODO: try to add -Wall and -Wextra to compile options to clean up more warnings