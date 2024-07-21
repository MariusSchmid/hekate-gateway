
# getting started
Dev Docker container are used

# git submodules

git submodule add https://github.com/FreeRTOS/FreeRTOS-Kernel.git
git submodule add https://github.com/rxi/log.c.git 
git submodule update --init --recursive


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
    * usbipd list
    * usbipd bind --hardware-id  2e8a:000c 
    * usbipd attach --hardware-id  2e8a:000c 


# environment variables
~~~
wsl
vi .profile
export WIFI_PWD=xyz
export WIFI_SSID=xyz
~~~
