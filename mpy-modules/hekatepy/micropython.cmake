add_library(usermod_hekatepy INTERFACE)

target_sources(usermod_hekatepy INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/hekatepy.c
)

target_include_directories(usermod_hekatepy INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(usermod INTERFACE usermod_hekatepy)