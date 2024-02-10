#!/bin/bash
cmake -B build
cmake --build build
cd external/micropython/ports/unix
make submodules
make USER_C_MODULES=../../../../mpy-modules/
