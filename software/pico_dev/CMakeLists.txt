cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH /home/dev/pico-sdk/)
# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)
# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

set(CMAKE_BUILD_TYPE Debug)
# rest of your project
add_executable(hello_world
    hello_world.c
)

add_subdirectory(../concentrator concentrator)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(hello_world pico_stdlib concentrator hardware_spi)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(hello_world)