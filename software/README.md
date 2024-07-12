
# getting started
Dev Docker container are used



# git submodules

## init all submodules
git submodule init 
git submodule update



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
