

set(_PATH ${CMAKE_CURRENT_SOURCE_DIR}/linux)

add_executable(custom-gateway 
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
    ${_PATH}/station_main.c
    )

add_subdirectory(../external external)

target_include_directories(custom-gateway PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(custom-gateway PRIVATE basicstatic pthread)