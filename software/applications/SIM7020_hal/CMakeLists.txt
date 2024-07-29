cmake_minimum_required(VERSION 3.13)

    

set(PICO_SDK_PATH /home/dev/pico-sdk/)
# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)


project(sim7020_example)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()



set(CMAKE_BUILD_TYPE Debug)
# rest of your project
add_executable(sim7020_example
    main.c
)


target_sources(sim7020_example
    PRIVATE
    ../../src/sim7020/sim7020_hal.c
    ../../src/sim7020/sim7020.c
    ../../src/free_rtos_memory.c
)
target_include_directories(sim7020_example
    PRIVATE
    ../../src/sim7020/
)


add_subdirectory(../../external/log_c logc)
add_subdirectory(../../utils hekate_utils)

set(FREERTOS_CFG_PATH ${CMAKE_CURRENT_LIST_DIR})
add_subdirectory(../../external/freertos freertos)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(sim7020_example PRIVATE 
    pico_stdlib 
    hardware_spi 
    logc
    FreeRTOS
    hekate_utils
)

target_include_directories(sim7020_example PRIVATE .)

# enable usb output, disable uart output
pico_enable_stdio_usb(sim7020_example 1)
pico_enable_stdio_uart(sim7020_example 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(sim7020_example)
