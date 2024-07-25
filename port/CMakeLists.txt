# ioLibrary_Driver
add_library(IOLIBRARY_FILES STATIC)

target_sources(IOLIBRARY_FILES PUBLIC
        ${PORT_DIR}/ioLibrary_Driver/src/w5x00_spi.c
        ${PORT_DIR}/ioLibrary_Driver/src/w5x00_gpio_irq.c
        )

target_include_directories(IOLIBRARY_FILES PUBLIC
        ${WIZNET_DIR}/Ethernet
        ${PORT_DIR}/ioLibrary_Driver/inc
        ${PORT_DIR}
        )

target_link_libraries(IOLIBRARY_FILES PRIVATE
        pico_stdlib
        hardware_spi
        hardware_dma
        hardware_clocks
        )

add_library(WIZ_LWIP_FILES STATIC)

target_sources(WIZ_LWIP_FILES PUBLIC
        ${PORT_DIR}/lwip/w5x00_lwip.c
        )

target_include_directories(WIZ_LWIP_FILES PUBLIC
        ${PICO_EXTRAS_LWIP_SRC_DIR}
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/rtos
        ${PROJECT_SOURCE_DIR}/hardware/rp2040
        ${PROJECT_SOURCE_DIR}/driver_lib
        ${PROJECT_SOURCE_DIR}/services
        ${PROJECT_SOURCE_DIR}/littlefs/littlefs
        #${PROJECT_SOURCE_DIR}/git_version
        ${PROJECT_SOURCE_DIR}/libraries/ioLibrary_Driver/Ethernet
        ${PROJECT_SOURCE_DIR}/libraries/ioLibrary_Driver/Ethernet/W5500
        ${PROJECT_SOURCE_DIR}/port/ioLibrary_Driver/inc
        ${PROJECT_SOURCE_DIR}/port/lwip
        ${PROJECT_SOURCE_DIR}/port/pio/include
        ${PROJECT_SOURCE_DIR}/port
        ${PICO_SDK_PATH}/lib/lwip/src/include
        ${PICO_SDK_PATH}/src/boards/include/boards
        ${PICO_SDK_PATH}/src/rp2_common/pico_lwip/include
        ${PICO_SDK_PATH}/src/rp2_common/pico_rand/include
        ${PICO_SDK_PATH}/src/rp2_common/hardware_spi/include
        ${PICO_SDK_PATH}/src/rp2_common/hardware_dma/include
        )

target_link_libraries(WIZ_LWIP_FILES PRIVATE
        ETHERNET_FILES
        #lwip
        )

