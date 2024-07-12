#!/bin/bash 

cd ../concentrator/
cmake -B build_mpy_unix
cmake --build build_mpy_unix

#workaround to force a rebuild
touch mpy/concentratorpy/concentratorpy.c 

cd ../micropython
cd external/micropython/V1.23.0/micropython/ports/unix
make submodules
# make
make USER_C_MODULES=../../../../../../../concentrator/mpy/
# make USER_C_MODULES=../../../../../../mpy-modules/

cd ../../../../../../
pwd
./run-mpy-unix.sh