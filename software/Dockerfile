FROM alpine:3.20

# ENV PYTHONUNBUFFERED=1
# RUN apk add --update --no-cache python3 && ln -sf python3 /usr/bin/python
# RUN python3 -m ensurepip
# RUN pip3 install --no-cache --upgrade pip setuptools

RUN apk add --no-cache python3 py3-pip


RUN apk add gdb cmake gcc make g++ git bash

# git config
RUN git config --global user.email "mafloschmid@aol.com"
RUN git config --global user.name "Marius Schmid"

#micropython unix
RUN apk add libffi-dev libbsd-dev

#micropython rp2
RUN apk add gcc-arm-none-eabi newlib-arm-none-eabi g++-arm-none-eabi

# rp2 debugging
RUN apk add gdb-multiarch
RUN ln -s /usr/bin/gdb-multiarch /usr/bin/arm-none-eabi-gdb


# pico sdk
RUN mkdir /home/dev
RUN cd /home/dev && git clone https://github.com/raspberrypi/pico-sdk.git
RUN cd /home/dev && git submodule update --init


#build openocd
RUN cd /home/dev && git clone https://github.com/openocd-org/openocd.git
RUN apk add automake autoconf  texinfo libtool  libusb-dev
RUN cd /home/dev/openocd/ && ./bootstrap
RUN cd /home/dev/openocd/ && ./configure --enable-picoprobe
RUN cd /home/dev/openocd/ && make -j4
RUN cd /home/dev/openocd/ && make install


