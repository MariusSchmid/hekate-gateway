cmake_minimum_required(VERSION 3.13)

project(hekate_utils)

add_library(hekate_utils STATIC "utils.c")

target_include_directories(hekate_utils
    PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}
)

