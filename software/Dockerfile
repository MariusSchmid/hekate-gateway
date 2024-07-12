FROM alpine:3.16

ENV PYTHONUNBUFFERED=1
RUN apk add --update --no-cache python3 && ln -sf python3 /usr/bin/python
RUN python3 -m ensurepip
RUN pip3 install --no-cache --upgrade pip setuptools


RUN apk add gdb cmake gcc make g++ git bash

#micropython
# RUN apk add libffi-dev build-base bsd-compat-headers
RUN apk add libffi-dev libbsd-dev