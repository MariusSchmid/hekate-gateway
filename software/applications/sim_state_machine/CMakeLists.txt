cmake_minimum_required(VERSION 3.13)
project(sim_state_machine)

# rest of your project
add_executable(sim_state_machine
    main.c
    ../../src/sim7020_state_machine.c
)

# add_subdirectory(../../concentrator concentrator)

# target_link_libraries(sim_state_machine PRIVATE concentrator)