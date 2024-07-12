add_library(usermod_concentrator INTERFACE)

target_sources(usermod_concentrator INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/concentratorpy.c
)

target_include_directories(usermod_concentrator INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)


add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../ concentrator)
target_link_libraries(usermod_concentrator INTERFACE concentrator)
target_link_libraries(usermod INTERFACE usermod_concentrator)