cmake_minimum_required(VERSION 3.15)

project(CustomGateway)

add_executable(custom-gateway 
    sys.c
    sys_linux.c 
    sys_log.c
    leds.c
    gps.c
    cmdfifo.c
    commands.c
    ral_master.c
    rmtsh.c
    web_linux.c
    station_main.c
    )

add_subdirectory(../external external)

target_include_directories(custom-gateway PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(custom-gateway PRIVATE basicstatic pthread)