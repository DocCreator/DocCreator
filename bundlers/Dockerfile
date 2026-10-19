# DocCreator in a container, built from this source tree.
#
# From the root of the repository:
#   docker build -f bundlers/Dockerfile -t doccreator .
# cf bundlers/README.md to run it.
#
# B: the image was based on Ubuntu 20.04 (focal, out of support), with Qt 5
# and libc++, and built the "pybind11" branch downloaded as an unpinned zip
# from GitHub, not the sources it was next to. It is now Ubuntu 24.04, with
# Qt 6, and the tesseract of the distribution (whose models are found, cf
# cmake/Modules/FindTesseract.cmake): nothing is downloaded at configure
# time.

FROM ubuntu:24.04
LABEL maintainer="Boris Mansencal <boris.mansencal@labri.fr>"

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update -qy \
 && apt-get install -qy --no-install-recommends \
      cmake ninja-build g++ git ca-certificates \
      libopencv-dev qt6-base-dev libgl1-mesa-dev \
      libtesseract-dev libleptonica-dev tesseract-ocr-eng tesseract-ocr-fra \
 && rm -rf /var/lib/apt/lists/*

COPY . /src

RUN cmake -S /src -B /build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
 && cmake --build /build --parallel \
 && cmake --install /build \
 && rm -rf /build \
 && QT_QPA_PLATFORM=offscreen DocCreator --self-test

# Env vars for the nvidia-container-runtime.
ENV NVIDIA_VISIBLE_DEVICES=all
ENV NVIDIA_DRIVER_CAPABILITIES=graphics,utility,compute

CMD ["DocCreator"]
