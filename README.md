# hekate
Outdoor Lora Gateway with Pico Pi

# submodules
git submodule add "https://github.com/lorabasics/basicstation.git" external/basicstation

# custom gateway

## build
~~~
cd custom-gateway
cmake -B build
cmake --build build
~~~


# lora simulation
## build
~~~
cd external/basicstation/examples/simulation
pip install -r requirements.txt
make station
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
sudo apt-get install build-essential libffi-dev git pkg-config
~~~
cd ports/unix
make submodules
make
~~~