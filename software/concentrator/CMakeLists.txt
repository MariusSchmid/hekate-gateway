
cmake_minimum_required(VERSION 3.23)
project(concentrator)


add_library(concentrator STATIC )

set(_SOURCE_ROOT_PATH ${CMAKE_CURRENT_LIST_DIR}/external/sx1302_hal/V2.1.0/sx1302_hal/libloragw)
set(_LIBTOOLS_ROOT_PATH ${CMAKE_CURRENT_LIST_DIR}/external/sx1302_hal/V2.1.0/sx1302_hal/libtools)
set(_CUSTOM_ROOT_PATH ${CMAKE_CURRENT_LIST_DIR}/src/custom_sx1302)

target_sources(concentrator PRIVATE

    ${_CUSTOM_ROOT_PATH}/loragw_hal.c 

    ${_SOURCE_ROOT_PATH}/src/loragw_reg.c 
    ${_CUSTOM_ROOT_PATH}/loragw_i2c.c 
    ${_CUSTOM_ROOT_PATH}/loragw_spi.c 
    ${_CUSTOM_ROOT_PATH}/sx125x_spi.c 
    ${_CUSTOM_ROOT_PATH}/sx1250_spi.c 
    ${_CUSTOM_ROOT_PATH}/sx1261_spi.c 
    ${_CUSTOM_ROOT_PATH}/loragw_aux.c 
    ${_CUSTOM_ROOT_PATH}/custom_usb.c 
    ${_CUSTOM_ROOT_PATH}/custom_com.c 
    ${_CUSTOM_ROOT_PATH}/loragw_sx1302.c 

    ${_SOURCE_ROOT_PATH}/src/loragw_com.c 
    ${_SOURCE_ROOT_PATH}/src/sx125x_com.c 
    ${_SOURCE_ROOT_PATH}/src/sx1261_com.c 
    ${_SOURCE_ROOT_PATH}/src/sx1250_com.c 

    ${_SOURCE_ROOT_PATH}/src/loragw_sx1250.c 
    ${_SOURCE_ROOT_PATH}/src/loragw_sx1261.c 
    ${_SOURCE_ROOT_PATH}/src/loragw_sx125x.c 
    ${_SOURCE_ROOT_PATH}/src/loragw_cal.c 
    ${_SOURCE_ROOT_PATH}/src/loragw_lbt.c 
    ${_SOURCE_ROOT_PATH}/src/loragw_stts751.c 
    ${_SOURCE_ROOT_PATH}/src/loragw_sx1302_timestamp.c 
    ${_SOURCE_ROOT_PATH}/src/loragw_sx1302_rx.c 
    ${_SOURCE_ROOT_PATH}/src/loragw_ad5338r.c


    ${_LIBTOOLS_ROOT_PATH}/src/base64.c

    src/concentrator.c
    src/concentrator_spi.c
    src/lora_config.c
    src/wait.c
    src/semtech_packet.c
)

# target_compile_options(sx1302_hal PRIVATE -Os)
target_compile_definitions(concentrator 
    PRIVATE 
    DEBUG_COM=1
    DEBUG_REG=1
    DEBUG_HAL=1
    )

# target_link_libraries(sx1302_hal PRIVATE platform)

# Add the current directory as an include directory.
target_include_directories(concentrator 
    PUBLIC
    # ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${_LIBTOOLS_ROOT_PATH}/inc
    ${_SOURCE_ROOT_PATH}/inc
    ${_CUSTOM_ROOT_PATH}
)


option(ENABLE_EXAMPLE "Build example code" OFF)
message(STATUS "ENABLE_EXAMPLE: ${ENABLE_EXAMPLE}")
    if(ENABLE_EXAMPLE)
    add_executable(concentrator_example "")
    target_sources(concentrator_example 
        PRIVATE 
        example/main.c
    )
    target_link_libraries(concentrator_example PRIVATE concentrator)
endif()