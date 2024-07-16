
set(FREERTOS_SRC_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Kernel)



add_library(FreeRTOS STATIC
    ${FREERTOS_SRC_DIRECTORY}/event_groups.c
    ${FREERTOS_SRC_DIRECTORY}/list.c
    ${FREERTOS_SRC_DIRECTORY}/queue.c
    ${FREERTOS_SRC_DIRECTORY}/stream_buffer.c
    ${FREERTOS_SRC_DIRECTORY}/tasks.c
    ${FREERTOS_SRC_DIRECTORY}/timers.c
    ${FREERTOS_SRC_DIRECTORY}/portable/MemMang/heap_3.c
    ${FREERTOS_SRC_DIRECTORY}/portable/ThirdParty/GCC/RP2040/port.c
)

message(STATUS "${PICO_SDK_PATH}/src/boards/include/boards/")
target_include_directories(FreeRTOS PUBLIC
    ${FREERTOS_CFG_DIRECTORY}/
    ${FREERTOS_SRC_DIRECTORY}/include
    ${FREERTOS_SRC_DIRECTORY}/portable/ThirdParty/GCC/RP2040/include
    ${FREERTOS_CFG_PATH}
)

target_link_libraries(FreeRTOS PRIVATE pico_stdlib hardware_exception  ) 