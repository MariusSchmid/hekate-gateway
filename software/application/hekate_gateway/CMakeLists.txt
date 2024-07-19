cmake_minimum_required(VERSION 3.13)

include(Password.cmake)
if(DEFINED ENV{WIFI_PWD})
    message(STATUS "WIFI-PWD: $ENV{WIFI_PWD}")
    set(WIFI_PASSWORD $ENV{WIFI_PWD})
else()
    message(FATAL_ERROR "WIFI PWD not set")
endif()
    
if(DEFINED ENV{WIFI_SSID})
    message(STATUS "WIFI_SSID: $ENV{WIFI_SSID}")
    set(WIFI_SSID $ENV{WIFI_SSID})
else()
    message(FATAL_ERROR "WIFI_SSID not set")
endif()


set(PICO_SDK_PATH /home/dev/pico-sdk/)
# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)


project(hekate_gateway)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()



set(CMAKE_BUILD_TYPE Debug)
# rest of your project
add_executable(hekate_gateway
    main.c
    gateway_task.c
    packet_forwarder_task.c
)




add_subdirectory(../../concentrator concentrator)

set(FREERTOS_CFG_PATH ${CMAKE_CURRENT_LIST_DIR}/config)
add_subdirectory(../../external/freertos freertos)

# set(FREERTOS_KERNEL_PATH ../external/freertos/FreeRTOS-Kernel)
# include(FreeRTOS_Kernel_import.cmake)



# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(hekate_gateway PRIVATE 
    pico_stdlib 
    # pico_cyw43_arch_lwip_threadsafe_background 
    # pico_cyw43_arch_lwip_sys_freertos 
    pico_cyw43_arch_lwip_poll
    hardware_spi 
    concentrator 
    FreeRTOS
)

target_include_directories(hekate_gateway PRIVATE .)

# enable usb output, disable uart output
pico_enable_stdio_usb(hekate_gateway 1)
pico_enable_stdio_uart(hekate_gateway 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(hekate_gateway)


target_compile_definitions(hekate_gateway PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )