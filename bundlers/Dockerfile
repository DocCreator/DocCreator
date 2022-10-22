From ubuntu:focal
MAINTAINER Boris Mansencal boris.mansencal@labri.fr

ARG DEBIAN_FRONTEND=noninteractive
ARG DEBCONF_NONINTERACTIVE_SEEN=true

RUN apt-get update -qy
RUN apt install -qy libc++-dev libc++abi-dev libopencv-dev qtbase5-dev qtdeclarative5-dev libqt5xmlpatterns5-dev cmake wget unzip g++ git

RUN wget https://github.com/DocCreator/DocCreator/archive/pybind11.zip
RUN unzip pybind11.zip
RUN cd DocCreator-pybind11 && mkdir build && cd build && cmake .. && make

# Env vars for the nvidia-container-runtime.
ENV NVIDIA_VISIBLE_DEVICES all
ENV NVIDIA_DRIVER_CAPABILITIES graphics,utility,compute
