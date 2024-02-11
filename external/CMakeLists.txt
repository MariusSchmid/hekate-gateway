
add_library(basicstation INTERFACE )

set(_BASIC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/basicstation)

target_sources(basicstation INTERFACE
    ${_BASIC_PATH}/src/aio.c
    ${_BASIC_PATH}/src/log.c
    ${_BASIC_PATH}/src/rt.c
    ${_BASIC_PATH}/src/uj.c
    ${_BASIC_PATH}/src/lora.c
    ${_BASIC_PATH}/src/net.c
    ${_BASIC_PATH}/src/s2conf.c
    ${_BASIC_PATH}/src/s2e.c
    ${_BASIC_PATH}/src/xq.c
    ${_BASIC_PATH}/src/tc.c
    ${_BASIC_PATH}/src/cups.c
    ${_BASIC_PATH}/src/web.c
    ${_BASIC_PATH}/src/selftests.c
    ${_BASIC_PATH}/src/ral.c
    ${_BASIC_PATH}/src/ral_lgw.c
    ${_BASIC_PATH}/src/ral_lgw2.c
    ${_BASIC_PATH}/src/tls.c
    ${_BASIC_PATH}/src/timesync.c
    ${_BASIC_PATH}/src/flashsim.c
    ${_BASIC_PATH}/src/sx130xconf.c
)


target_include_directories(basicstation INTERFACE 
    ${_BASIC_PATH}/src
    ${_BASIC_PATH}/../../hekatelib
    ${CMAKE_CURRENT_SOURCE_DIR}
)


target_compile_definitions(basicstation INTERFACE
#if unix
    # CFG_linux
    CFG_no_leds
    CFG_version="2.0.6"
    CFG_platform="linux"
    CFG_bdate="today"
    CFG_lgw1
    CFG_ral_lgw
    CFG_lgwsim
)

if(MPY)
    target_include_directories(basicstation INTERFACE ${CMAKE_CURRENT_LIST_DIR}/micropython/lib/mbedtls/include)
else()
    target_sources(basicstation PRIVATE
    ${_BASIC_PATH}/src/fs.c
    ${_BASIC_PATH}/src/lgwsim.c
    ${_BASIC_PATH}/src/sys.c
    ${_BASIC_PATH}/src-linux/sys_linux.c 
    ${_BASIC_PATH}/src-linux/sys_log.c
    ${_BASIC_PATH}/src-linux/leds.c
    ${_BASIC_PATH}/src-linux/gps.c
    ${_BASIC_PATH}/src-linux/cmdfifo.c
    ${_BASIC_PATH}/src-linux/commands.c
    ${_BASIC_PATH}/src-linux/ral_master.c
    ${_BASIC_PATH}/src-linux/rmtsh.c
    ${_BASIC_PATH}/src-linux/web_linux.c

    )

    target_include_directories(basicstation PUBLIC 
        ${_BASIC_PATH}/src-linux
    )

    target_compile_definitions(basicstation PUBLIC
        CFG_linux
    )


    cmake_policy(SET CMP0077 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    set(ENABLE_PROGRAMS OFF)
    set(ENABLE_TESTING OFF)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/basicstation/deps/mbedtls/platform-linux mbedtls)
    target_link_libraries(basicstation PUBLIC mbedtls m pthread)
endif()