set(_PATH ${CMAKE_CURRENT_LIST_DIR}/src)

set(_SOURCES 
    src/hekate.c
)

if(MPY)

list(APPEND _SOURCES 
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
    #common
    # ${CMAKE_CURRENT_LIST_DIR}/common/sys_linux.c 
    # ${CMAKE_CURRENT_LIST_DIR}/common/sys.c 
)
endif()

set(_INC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/common
)

add_subdirectory(../external external)



add_library(hekatelib INTERFACE)
target_sources(hekatelib INTERFACE 
    ${_SOURCES}
)
target_include_directories(hekatelib INTERFACE 
    ${_INC}
)
target_link_libraries(hekatelib INTERFACE basicstation)


add_library(hekatelib_static STATIC "")
target_sources(hekatelib_static PRIVATE 
    ${_SOURCES}
)
target_include_directories(hekatelib_static PUBLIC 
    ${_INC}
)
target_link_libraries(hekatelib_static PUBLIC basicstation)

if(NOT ${MPY})
if(${BUILD_EXECUTABLE})
    message(STATUS "Build Executable")
    add_executable(hekatebin
    ${CMAKE_CURRENT_LIST_DIR}/linux/station_main.c
    )
    target_link_libraries(hekatebin PUBLIC hekatelib_static)
endif()
endif()