#!/bin/bash
echo "test"

cd external/micropython/ports/unix
make USER_C_MODULES=../../../../mpy-modules/
