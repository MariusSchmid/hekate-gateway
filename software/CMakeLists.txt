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






# initialize the Raspberry Pi Pico SDK
set(PICO_SDK_PATH /home/dev/pico-sdk/)
include(src/pico_sdk_import.cmake)
pico_sdk_init()

project(hekate_gateway)

add_subdirectory(utils)
add_subdirectory(concentrator)

set(FREERTOS_CFG_PATH ${CMAKE_CURRENT_LIST_DIR}/src/config)
add_subdirectory(external/freertos)
add_subdirectory(external/log_c)

#print memory usage
add_link_options(-Wl,--print-memory-usage)

set(CMAKE_BUILD_TYPE Debug)
# rest of your project
add_executable(hekate_gateway
    src/main.c
    src/lora_task.c
    src/packet_forwarder_task.c
    src/time_ntp.c
    src/free_rtos_memory.c
    $<$<BOOL:${INTERNET_SIM}>:src/sim7020_internet_task.c>
    $<$<BOOL:${INTERNET_WIFI}>:src/wifi_internet_task.c>
)



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
    src
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
        )

