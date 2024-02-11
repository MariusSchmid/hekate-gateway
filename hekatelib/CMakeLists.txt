set(_PATH ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM})

set(_SOURCES 
    # ${_PATH}/sys.c
    # ${_PATH}/sys_linux.c 
    ${_PATH}/sys_log.c
    ${_PATH}/leds.c
    ${_PATH}/gps.c
    ${_PATH}/cmdfifo.c
    ${_PATH}/commands.c
    ${_PATH}/ral_master.c
    ${_PATH}/rmtsh.c
    ${_PATH}/web_linux.c
    src/hekate.c
    #common
    ${CMAKE_CURRENT_LIST_DIR}/common/sys_linux.c 
    ${CMAKE_CURRENT_LIST_DIR}/common/sys.c 
)

set(_INC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/common/include

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


add_library(hekatelib_static STATIC)
target_sources(hekatelib_static PRIVATE 
    ${_SOURCES}
)
target_include_directories(hekatelib_static PUBLIC 
    ${_INC}
)
target_link_libraries(hekatelib_static PUBLIC basicstation)


if(${BUILD_EXECUTABLE})
    message(STATUS "Build Executable")
    add_executable(hekatebin
        ${_PATH}/station_main.c
    )
    target_link_libraries(hekatebin PUBLIC hekatelib pthread)
endif()