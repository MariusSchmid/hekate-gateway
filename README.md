# hekate
Outdoor Lora Gateway with Pico Pi

# submodules
git submodule add "https://github.com/lorabasics/basicstation.git" external/basicstation

# how to build lora simulation
in wsl:
~~~
cd external/basicstation/examples/simulation
pip install -r requirements.txt
make station
~~~

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