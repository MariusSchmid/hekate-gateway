cmake_minimum_required(VERSION 3.13)
project(gateway_packets_example)

# rest of your project
add_executable(gateway_packets_example
    main.c
)

add_subdirectory(../../concentrator concentrator)

target_link_libraries(gateway_packets_example PRIVATE concentrator)