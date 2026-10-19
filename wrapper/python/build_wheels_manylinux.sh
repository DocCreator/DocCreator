#!/bin/bash
#
# Build the wheels of the python module in a manylinux container, for each
# CPython of the container, and repair them with auditwheel.
#
# From the root of the repository:
#   docker run --rm -v "$PWD":/io quay.io/pypa/manylinux_2_28_x86_64 \
#     /io/wrapper/python/build_wheels_manylinux.sh [--osmesa]
#
# The wheels are written to WHEELS/ (WHEELS_OSMESA/ with --osmesa), at the
# root of the repository. PYTHON_VERSIONS chooses the interpreters, as tags
# of /opt/python separated by spaces (for example "cp312-cp312"); by
# default, every CPython 3.9 or later that is not free-threaded.
#
# B: the script was out of date: it targeted manylinux2010 and OpenCV 4.5.1,
# installed no "build" module while the package is made with
# "python -m build", gave the interpreter as PYTHON_* variables that
# FindPython ignores, tried every /opt/python (pypy and free-threaded
# builds too), downloaded with no hash check, and went on after a failure.

set -euo pipefail

WITH_OSMESA=OFF
SUFFIX=""
if [ "${1:-}" = "--osmesa" ] ; then
    WITH_OSMESA=ON
    SUFFIX="_OSMESA"
fi

SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
WHEELS_DIR="${SRC_DIR}/WHEELS${SUFFIX}"
BUILD_ROOT_DIR="${SRC_DIR}/BUILD_PYTHON${SUFFIX}"
mkdir -p "${WHEELS_DIR}" "${BUILD_ROOT_DIR}"
cd "${BUILD_ROOT_DIR}"

OPENCV_VERSION=5.0.0
#SHA256 of ${OPENCV_VERSION}.tar.gz: change it when the version changes
OPENCV_SHA256=b0528f5a1d379d59d4701cb28c36e22214cc51cf64594e5b56f2d3e6c0233095
JOBS=$(nproc)

### tools: cmake and ninja of a recent python of the container
TOOLS_PY=$(ls -d /opt/python/cp3*-cp3* | grep -v 't$' | sort -V | tail -1)/bin/python
"${TOOLS_PY}" -m pip install --upgrade cmake ninja
export PATH="$(dirname "${TOOLS_PY}"):${PATH}"

### OSMesa, from the distribution of the container
if [ "${WITH_OSMESA}" = "ON" ] ; then
    dnf install -y mesa-libOSMesa-devel
fi

### OpenCV, the modules the degradations use
OPENCV_PREFIX="${BUILD_ROOT_DIR}/opencv-install"
if [ ! -f "${OPENCV_PREFIX}/.built" ] ; then
    curl -sSL -o opencv.tar.gz "https://github.com/opencv/opencv/archive/refs/tags/${OPENCV_VERSION}.tar.gz"
    echo "${OPENCV_SHA256}  opencv.tar.gz" | sha256sum -c -
    rm -rf "opencv-${OPENCV_VERSION}" opencv-build
    tar xzf opencv.tar.gz
    cmake -S "opencv-${OPENCV_VERSION}" -B opencv-build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${OPENCV_PREFIX}" \
        -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
        -DBUILD_LIST=core,imgproc,imgcodecs,highgui,photo,geometry \
        -DENABLE_FAST_MATH=ON -DBUILD_DOCS=OFF -DBUILD_PERF_TESTS=OFF \
        -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_opencv_apps=OFF \
        -DBUILD_JAVA=OFF -DBUILD_opencv_python3=OFF -DWITH_CUDA=OFF \
        -DWITH_FFMPEG=OFF -DWITH_GTK=OFF -DWITH_QT=OFF -DWITH_OPENCL=OFF \
        -DWITH_OPENEXR=OFF -DWITH_OPENJPEG=OFF -DWITH_JASPER=OFF \
        -DWITH_WEBP=OFF -DWITH_AVIF=OFF -DWITH_VTK=OFF -DWITH_EIGEN=OFF \
        -DWITH_PROTOBUF=OFF -DWITH_IPP=OFF \
        -DBUILD_ZLIB=ON -DBUILD_PNG=ON -DBUILD_JPEG=ON -DBUILD_TIFF=ON
    cmake --build opencv-build --parallel "${JOBS}"
    cmake --install opencv-build
    touch "${OPENCV_PREFIX}/.built"
fi
OPENCV_DIR=$(dirname "$(find "${OPENCV_PREFIX}" -name OpenCVConfig.cmake | head -1)")

### the wheels
if [ -z "${PYTHON_VERSIONS:-}" ] ; then
    PYTHON_VERSIONS=""
    for PYROOT in /opt/python/cp3*-cp3* ; do
        PYTAG=$(basename "${PYROOT}")
        case "${PYTAG}" in
            (*t) continue ;;                       #free-threaded
            (cp36-*|cp37-*|cp38-*) continue ;;     #end of life
        esac
        PYTHON_VERSIONS="${PYTHON_VERSIONS} ${PYTAG}"
    done
fi

for PYTAG in ${PYTHON_VERSIONS} ; do
    echo "=== ${PYTAG}"
    PYBIN="/opt/python/${PYTAG}/bin"
    VENV="${BUILD_ROOT_DIR}/venv_${PYTAG}"
    rm -rf "${VENV}"
    "${PYBIN}/python" -m venv "${VENV}"
    "${VENV}/bin/python" -m pip install --upgrade pip
    "${VENV}/bin/python" -m pip install numpy build setuptools wheel pytest

    BUILD_DIR="${BUILD_ROOT_DIR}/build_${PYTAG}"
    rm -rf "${BUILD_DIR}"
    cmake -S "${SRC_DIR}" -B "${BUILD_DIR}" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_ONLY_DEGRADATIONS=ON -DBUILD_PYTHON_WRAPPER=ON \
        -DBUILD_WITH_OSMESA=${WITH_OSMESA} \
        -DOpenCV_DIR="${OPENCV_DIR}" \
        -DPython_EXECUTABLE="${VENV}/bin/python"
    cmake --build "${BUILD_DIR}" --parallel "${JOBS}"

    rm -rf "${BUILD_DIR}/wheelhouse"
    auditwheel repair -w "${BUILD_DIR}/wheelhouse" "${BUILD_DIR}"/wrapper/python/PACKAGE/dist/*.whl

    #the repaired wheel must import and pass the tests, out of the build tree
    "${VENV}/bin/python" -m pip install "${BUILD_DIR}"/wheelhouse/*.whl
    (cd /tmp && "${VENV}/bin/python" -m pytest -q "${SRC_DIR}/tests/python")

    cp "${BUILD_DIR}"/wheelhouse/*.whl "${WHEELS_DIR}/"
done

echo "wheels in ${WHEELS_DIR}:"
ls -l "${WHEELS_DIR}"
