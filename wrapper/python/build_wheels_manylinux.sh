
WHEELS_DIR="${PWD}/WHEELS"
mkdir -p ${WHEELS_DIR}

BUILD_ROOT_DIR="${PWD}/BUILD_PYTHON"
mkdir -p ${BUILD_ROOT_DIR}
cd ${BUILD_ROOT_DIR}


###build dependencies

ORIGINAL_PATH=$PATH
export PATH=/opt/python/cp38-cp38/bin:$PATH
pip install cmake   #install cmake 3.18.2

###build opencv
curl -LO https://github.com/opencv/opencv/archive/4.5.1.tar.gz
tar xzf 4.5.1.tar.gz
cd opencv-4.5.1/
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=${PWD}/install -DBUILD_opencv_highgui=ON -DENABLE_FAST_MATH=ON -DBUILD_DOCS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_TESTS=OFF -DWITH_CUDA=OFF -DWITH_CUFFT=OFF -DWITH_FFMPEG=OFF -DWITH_GIGEAPI=OFF -DWITH_JASPER=OFF -DWITH_LIBV4L=OFF -DWITH_MATLAB=OFF -DWITH_OPENCL=OFF -DWITH_OPENCLAMDBLAS=OFF -DWITH_OPENCLAMDFFT=OFF -DWITH_OPENEXR=OFF -DWITH_PVAPI=OFF -DWITH_V4L=OFF -DWITH_VTK=OFF -DWITH_WEBP=OFF -DWITH_1394=OFF -DBUILD_opencv_apps=OFF -DBUILD_opencv_calib3d=OFF -DBUILD_opencv_dnn=OFF -DBUILD_opencv_features2d=OFF -DBUILD_opencv_flann=OFF -DBUILD_opencv_ml=OFF -DBUILD_opencv_objdetect=OFF -DBUILD_opencv_shape=OFF -DBUILD_opencv_stitching=OFF -DBUILD_opencv_superres=OFF -DBUILD_opencv_ts=OFF -DBUILD_opencv_video=OFF -DBUILD_opencv_videoio=OFF -DBUILD_opencv_videostab=OFF -DBUILD_opencv_world=OFF -DBUILD_opencv_highgui=ON -DBUILD_opencv_gapi=OFF -DBUILD_opencv_python2=OFF -DBUILD_opencv_python3=ON -DWITH_QUICKTIME=OFF -DHAVE_QTKIT=FALSE
make -j 3
make install
cd ../..



SKIP_PLATFORMS=(cp27-cp27m cp27-cp27mu)
#SKIP_PLATFORMS=(cp27-cp27m cp27-cp27mu cp35-cp35m cp36-cp36m cp37-cp37m cp39-cp39)  #DEBUG
for PYROOT in /opt/python/*; do
  PYTAG=$(basename "${PYROOT}")
  echo "$PYTAG"
  # Check for platforms to be skipped
  # shellcheck disable=SC2199,SC2076
  if [[ " ${SKIP_PLATFORMS[@]} " =~ " ${PYTAG} " ]]; then
    echo "skipping deprecated platform $PYTAG"
    continue
  fi

  PYBIN="${PYROOT}/bin"
  "${PYBIN}/pip" install virtualenv
  "${PYBIN}/virtualenv" -p "${PYBIN}/python" "venv_${PYTAG}"
  # shellcheck source=/dev/null
  source "venv_${PYTAG}/bin/activate"
  #pip install -U pip setuptools wheel

  # Clean the build dir
  BUILD_DIR="build_$PYTAG"
  rm -rf "${BUILD_DIR}"
  
  #LIB="${PYROOT}/lib/libpython$(python -V | grep -o "[0-9]\+.[0-9]\+.[0-9]\+").so"
  #touch "$LIB"

  pip install cmake
  pip install numpy

  PYTHONVER="$(${PYBIN}/python -V | grep -o "[0-9]\+.[0-9]\+")"  #keep 3.9 for 3.9.0
  INCLUDE1="${PYROOT}/include/python${PYTHONVER}"
  #INCLUDE2="${PYROOT}/lib/python${PYTHONVER}/site-packages/numpy/core/include"  #without virtualenv
  INCLUDE2="${PWD}/venv_${PYTAG}/lib/python${PYTHONVER}/site-packages/numpy/core/include" #with virtualenv
  
  mkdir ${BUILD_DIR}
  cd ${BUILD_DIR}
  
  OpenCV_DIR=${PWD}/../opencv-4.5.1/build/install/lib64/cmake/opencv4/ cmake ../.. -DBUILD_ONLY_DEGRADATIONS=ON -DBUILD_PYTHON_WRAPPER=ON -DPYTHON_INCLUDE_DIR="${INCLUDE1};${INCLUDE2}" -DPYTHON_EXECUTABLE="${PYBIN}/python"
  make VERBOSE=1
  pushd wrapper/python/PACKAGE
  auditwheel -v repair dist/*

  cp wheelhouse/* ${WHEELS_DIR}/

  popd
  cd ..
  
  # Restore environment
  deactivate
done
  
export PATH=$ORIGINAL_PATH
cd ..

