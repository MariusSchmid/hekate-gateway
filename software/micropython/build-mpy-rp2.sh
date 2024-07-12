#!/bin/bash



cd external/micropython/V1.23.0/micropython
make -C ports/rp2 submodules
make -C mpy-cross
cd ports/rp2

make USER_C_MODULES=../../../../../../../concentrator/mpy/micropython.cmake -j4
