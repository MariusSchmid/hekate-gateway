
# getting started
Dev Docker container are used

# git submodules
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


# RP2 debugging
## Pass USB device to WSL / Dev Container
* install https://github.com/dorssel/usbipd-win/releases
* in ADMIN CMD:
    * list all usb devices: usbipd list
    * usbipd bind --busid 5-2
    * usbipd attach --wsl --busid 5-2



# 