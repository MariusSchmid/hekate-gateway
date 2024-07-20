
set(LOGC_SRC_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/log.c)



add_library(logc STATIC
    ${LOGC_SRC_DIRECTORY}/src/log.c

)

target_include_directories(logc PUBLIC
    ${LOGC_SRC_DIRECTORY}/src
)

target_compile_definitions(logc PUBLIC LOG_USE_COLOR)