#!/bin/bash
# cmake -B build
# cmake --build build
cd external/micropython
make -C ports/rp2 submodules
make -C mpy-cross
cd ports/rp2

# make USER_C_MODULES=../../../../mpy-modules/
make USER_C_MODULES=../../../../mpy-modules/micropython.cmake
