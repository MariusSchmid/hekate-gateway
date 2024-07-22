cmake_minimum_required(VERSION 3.13)


option(INTERNET_WIFI "Use WiFi for Internet connection" OFF)
message(STATUS "INTERNET_WIFI: ${INTERNET_WIFI}")
option(INTERNET_SIM "Use Sim Module for Internet connection" OFF)
message(STATUS "INTERNET_SIM: ${INTERNET_SIM}")

if(INTERNET_SIM AND INTERNET_WIFI)
    message(FATAL_ERROR "INTERNET_SIM and INTERNET_WIFI can not be enabled the same time.")
endif()

if(INTERNET_WIFI)
    if(DEFINED ENV{WIFI_PWD})
        message(STATUS "WIFI-PWD: $ENV{WIFI_PWD}")
        set(WIFI_PASSWORD $ENV{WIFI_PWD})
    else()
        message(FATAL_ERROR "WIFI_PWD not set as environment variable.")
    endif()
        
    if(DEFINED ENV{WIFI_SSID})
        message(STATUS "WIFI_SSID: $ENV{WIFI_SSID}")
        set(WIFI_SSID $ENV{WIFI_SSID})
    else()
        message(FATAL_ERROR "WIFI_SSID not set as environment variable")
    endif()
endif()



set(PICO_SDK_PATH /home/dev/pico-sdk/)
include(pico_sdk_import.cmake)
pico_sdk_init()

project(hekate_gateway)

# initialize the Raspberry Pi Pico SDK




set(CMAKE_BUILD_TYPE Debug)
# rest of your project
add_executable(hekate_gateway
    main.c
    gateway_task.c
    packet_forwarder_task.c
    time_ntp.c
    $<$<BOOL:${INTERNET_SIM}>:sim7020_internet_task.c>
    $<$<BOOL:${INTERNET_WIFI}>:wifi_internet_task.c>
)






# set(FREERTOS_KERNEL_PATH ../external/freertos/FreeRTOS-Kernel)
# include(FreeRTOS_Kernel_import.cmake)

# add_compile_definitions(PICO_HEAP_SIZE=0x1000)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(hekate_gateway PRIVATE 
    pico_stdlib 
    pico_cyw43_arch_lwip_poll
    hardware_spi 
    concentrator 
    FreeRTOS
    logc
    hekate_utils
)

target_include_directories(hekate_gateway 
    PRIVATE 
    .
)

# enable usb output, disable uart output
pico_enable_stdio_usb(hekate_gateway 1)
pico_enable_stdio_uart(hekate_gateway 0)

# # create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(hekate_gateway)


target_compile_definitions(hekate_gateway PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        $<$<BOOL:${INTERNET_SIM}>:INTERNET_SIM>
        $<$<BOOL:${INTERNET_WIFI}>:INTERNET_WIFI>
        --print-memory-usage 
        )


