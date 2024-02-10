


set(_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM})

add_library(hekatelib STATIC)

add_subdirectory(../external external)

target_sources(hekatelib PUBLIC 
    ${_PATH}/sys.c
    ${_PATH}/sys_linux.c 
    ${_PATH}/sys_log.c
    ${_PATH}/leds.c
    ${_PATH}/gps.c
    ${_PATH}/cmdfifo.c
    ${_PATH}/commands.c
    ${_PATH}/ral_master.c
    ${_PATH}/rmtsh.c
    ${_PATH}/web_linux.c
    src/hekate.c
)


target_include_directories(hekatelib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(hekatelib PUBLIC basicstation)


if(${BUILD_EXECUTABLE})
    message(STATUS "Build Executable")
    add_executable(hekatebin
        ${_PATH}/station_main.c
    )
    target_link_libraries(hekatebin PUBLIC hekatelib pthread)
endif()