# hekate
Outdoor Lora Gateway with Pico Pi.

Full Documentation: [readthedocs]( https://hekate-gateway.readthedocs.io/en/latest/)

# Build instructions

## submodules
git submodule add "https://github.com/lorabasics/basicstation.git" external/basicstation

## prerequisites
git submodule init 
git submodule update

~~~
cd external/basicstation/examples/simulation
pip install -r requirements.txt
make station
~~~


# custom gateway

## to use vs code for debugging
* open wsl terminal
~~~
code .
~~~

## build
~~~
cd custom-gateway
cmake -B build
cmake --build build
~~~



## basicstation

What is part of platform layer (sys)?

src-linux/sys_linux #main
src/sys.c #socket options
src/lgwsim.c #socket write
sys_log.c #thread for logging

build-linux-testsim #header file is generated


Radio Layer (RAL)



# Troubleshooting


~~~
./prep.sh: line 2: $'\r': command not found
~~~

Solution:
~~~
sudo apt install dos2unix
dos2unix external/basicstation/examples/cups/prep.sh
dos2unix external/basicstation/deps/lgw/prep.sh
dos2unix external/basicstation/deps/mbedtls/prep.sh
~~~



# MicroPython

## unix port
prerequisites
~~~
sudo apt-get install build-essential libffi-dev git pkg-config
~~~

~~~
#build for unix
build-mpy-unix.sh
~~~


## rp2
prerequisites
~~~
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential
~~~

~~~
cd external/micropython
make -C mpy-cross
~~~

## run micropython
~~~
./external/micropython/ports/unix/build-standard/micropython
import hekate
hekate.init()
~~~



# need to build for
unix # normal cmake
mpy unix #shell script
mpy rp2 #shell script
