
cmake_minimum_required(VERSION 3.15)
project(CustomGateway)


set(PLATFORM "linux")
set(BUILD_EXECUTABLE ON)

cmake_policy(SET CMP0077 NEW)
add_subdirectory(hekatelib)

