#!/bin/bash
cd external/micropython
make -C ports/rp2 submodules
make -C mpy-cross
cd ports/rp2

make USER_C_MODULES=../../../../mpy-modules/micropython.cmake -j4
