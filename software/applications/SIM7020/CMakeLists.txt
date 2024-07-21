cmake_minimum_required(VERSION 3.13)

    

set(PICO_SDK_PATH /home/dev/pico-sdk/)
# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)


project(sim7020e)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()



set(CMAKE_BUILD_TYPE Debug)
# rest of your project
add_executable(sim7020e
    main.c
)

add_subdirectory(../../external/log_c logc)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(sim7020e PRIVATE 
    pico_stdlib 
    hardware_spi 
    logc
)

target_include_directories(sim7020e PRIVATE .)

# enable usb output, disable uart output
pico_enable_stdio_usb(sim7020e 1)
pico_enable_stdio_uart(sim7020e 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(sim7020e)
