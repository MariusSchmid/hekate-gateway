#!/bin/bash 
cmake -B build_mpy_unix -DMPY=ON
cmake --build build_mpy_unix

#workaround to force a rebuild
touch mpy-modules/hekatepy/hekatepy.c 

cd external/micropython/ports/unix
make submodules
make USER_C_MODULES=../../../../mpy-modules/

cd ../../../../
./run-mpy-unix.sh