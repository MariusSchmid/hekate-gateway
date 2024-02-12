add_library(usermod_hekatepy INTERFACE)

target_sources(usermod_hekatepy INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/hekatepy.c
)

target_include_directories(usermod_hekatepy INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)
set(PLATFORM "mpy")
set(MPY ON)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../hekatelib hekatelib)
target_link_libraries(usermod_hekatepy INTERFACE hekatelib)
target_link_libraries(usermod INTERFACE usermod_hekatepy)