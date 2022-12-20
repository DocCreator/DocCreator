# DocCreator [![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0) 
[![Build Status](https://api.travis-ci.org/DocCreator/DocCreator.svg?branch=master)](https://travis-ci.org/DocCreator/DocCreator)


 [DocCreator](https://doc-creator.labri.fr/) is an open source, cross-platform software allowing to generate synthetic document images and the accompanying groundtruth. Various degradation models can be applied on original document images to create virtually unlimited amounts of different images.

 Citation: if you use DocCreator in Reaserch work for publication, please cite:  
 **Journet, N.; Visani, M.; Mansencal, B.; Van-Cuong, K.; Billy, A.**  
 **DocCreator: A New Software for Creating Synthetic Ground-Truthed Document Images.**  
 **J. Imaging 2017, 3, 62.**   
 http://www.mdpi.com/2313-433X/3/4/62




## Dependencies

The program has the following dependencies :
* **OpenCV** (3.x or 4.x)  
* **Qt 5 or 6**  
* **Tesseract** [optional] If Tesseract is not found on the system, it will be compiled from (provided) sources. 
However, you will need to have network access during the configuration step to download tessdata, tesseract languages data.
* **OSMesa** [optional] for OpenGL offscreen rendering for the 3D distortion model.
* **CMake** is used for compilation configuration.
* **Ninja** [optional]. On Windows in particular, it may be convenient to install ninja to build all from the command line. 
* **git** (1.6.5 or later) [for tesseract download from CMake]
* ***C++ compiler*** This program should compile on linux (with gcc & clang), Mac OS (with clang) and Microsoft Windows 10 (with Visual Studio 2017).
It has been tested on Fedora (19->32), Ubuntu (14.04->20.04), Mac OS (10.9->10.15), Windows 10.


### Linux

On linux, your distribution may provide the binary packages for all required dependencies.

On Ubuntu (14.04 and above), you can install the required binary packages with the following command:  
`sudo apt-get install libopencv-dev qtbase5-dev qtdeclarative5-dev libqt5xmlpatterns5-dev cmake g++ git`  
You may also have to set Qt5 as default with:  
`sudo apt-get install qt5-default`  
On Ubuntu (22.04 and above), you can install the required binary packages with the following command:  
`sudo apt-get install libopencv-dev qt6-base-dev qt6-declarative-dev cmake g++ git`  


On Fedora (21 or 22), you can install the required binary packages with the following command as root:  
`yum install opencv-devel qt5-qtbase-devel qt5-qtxmlpatterns-devel cmake`  
On Fedora (23 and above), you can install the required binary packages with the following command as root for Qt5:   
`dnf install opencv-devel qt5-qtbase-devel qt5-qtxmlpatterns-devel cmake gcc-c++`  
or for Qt6:   
`dnf install opencv-devel qt6-qtbase-devel cmake gcc-c++`  


To compile with clang on linux:
- you need to have libc++  
  On Ubuntu, you can install libc++ with the following command:  
  `sudo apt-get install libc++-dev libc++abi-dev`  
  Warning: on Ubuntu 16.04, you may have to edit /usr/include/c++/v1/string
  cf https://stackoverflow.com/questions/37096062/get-a-basic-c-program-to-compile-using-clang-on-ubuntu-16/38385967#38385967
  On Fedora, you can use the following command:  
  `dnf install libcxx`  
- you need to have OpenCV 3.1 or above.
  See below how to build OpenCV from sources.

### CMake

On linux, CMake is often provided by the distribution.  
Binary packages of CMake last versions are available from https://cmake.org/download/  
**cmake must be in your PATH**. You should be able to print cmake version with the following commande: `cmake --version`

### Ninja

[Optionnal]  
Binary packages of Ninja are available from https://github.com/ninja-build/ninja/releases  
**ninja must be in your PATH**. You should be able to print ninja version with the following commande: `ninja --version`

 To add ninja to your path on Windows, you may open a cmd prompt "CMD.exe" with Administrator privileges (right click), and do:  
 `setx -m PATH "%PATH%;<PATH_to_ninja>"`  

 For macOS, Ninja binary packages (ninja-mac.zip) are also available from https://github.com/ninja-build/ninja/releases
 You will have to copy the ninja executable to a directory in your path (for example /usr/local/bin)
 On Mac OS 10.15 Catalina, you may also have to specify that ninja is not a malware.
 Run `ninja --version` (you should have a message preventing the execution). Go to "Apple menu" -> "System Preferences", click "Security & Privacy", then click "General". You should see "ninja" : click "open anyway"
 Run `ninja --version` a second time, you should have a dialog where you can choose "open".

 For Ubuntu:
 `sudo apt-get install ninja-build`


### Mac

On OSX (10.9, 10.10 or 10.11) with [Homebrew](https://brew.sh/), you can install the required dependencies with the following commands:
`brew install qt5`  
`brew linkapps qt5`  
`brew link --force qt5`  
`brew tap homebrew/science`  
`brew install opencv`  


### Qt

On linux, it is recommanded to use packages provided by the distribution. See above for instructions.  
On Mac and Windows, the open source version of Qt may be installed with the Qt Download Installer, downloaded from
https://www.qt.io/download  
On Windows, you will have to choose the version to install according to the compiler that you use and the target architecture. 
For example, "MSVC 2017 64-bit" if you use Microsoft Visual Studio 2017 on a 64-bit processor.


### OpenCV from sources

You can also install OpenCV from sources (if it is not provided by your distribution or if you want a newer version for example).

OpenCV is also built with CMake.
You can use the following command to configure a *minimal* OpenCV version compatible with DocCreator:
```
cmake <PATH_TO_CMakeLists.txt> -DCMAKE_INSTALL_PREFIX=<INSTALL_PREFIX> -DBUILD_opencv_highgui=ON -DENABLE_FAST_MATH=ON -DBUILD_DOCS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_TESTS=OFF -DWITH_CUDA=OFF -DWITH_CUFFT=OFF -DWITH_FFMPEG=OFF -DWITH_GIGEAPI=OFF -DWITH_JASPER=OFF -DWITH_LIBV4L=OFF  -DWITH_MATLAB=OFF -DWITH_OPENCL=OFF -DWITH_OPENCLAMDBLAS=OFF -DWITH_OPENCLAMDFFT=OFF -DWITH_OPENEXR=OFF -DWITH_PVAPI=OFF -DWITH_V4L=OFF -DWITH_VTK=OFF -DWITH_WEBP=OFF -DWITH_1394=OFF -DBUILD_opencv_apps=OFF -DBUILD_opencv_calib3d=OFF -DBUILD_opencv_dnn=OFF -DBUILD_opencv_features2d=OFF -DBUILD_opencv_flann=OFF -DBUILD_opencv_ml=OFF -DBUILD_opencv_objdetect=OFF -DBUILD_opencv_shape=OFF -DBUILD_opencv_stitching=OFF -DBUILD_opencv_superres=OFF  -DBUILD_opencv_ts=OFF -DBUILD_opencv_video=OFF -DBUILD_opencv_videoio=ON -DBUILD_opencv_videostab=OFF -DBUILD_opencv_world=OFF -DBUILD_opencv_highgui=ON -DPYTHON2_EXECUTABLE="" -DPYTHON3_EXECUTABLE="" -DBUILD_opencv_python2=OFF -DBUILD_opencv_python3=OFF -DWITH_QUICKTIME=OFF -DHAVE_QTKIT=FALSE
```
with corrected <PATH_TO_CMakeLists.txt> and <INSTALL_PREFIX>.

 #### On Windows

Binaries for Visual Studio are available from [OpenCV github releases page](https://github.com/opencv/opencv/releases).
However, you can also build it with Visual Studio IDE or from the command line with ninja.

  ##### With IDE

CMake can generate a .sln solution file to open with Visual Studio.

First, open a "Developer Command Prompt" ("Developer Command Prompt for VS 2017" for VS17 for example).  
`cd <OpenCV_SOURCES_DIRECTORY>`  
`mkdir build`  
`cd build`  

For Visual Studio 2017 x64, you can use the following command to generate a .sln file:
```
cmake <PATH_TO_CMakeLists.txt> -G "Visual Studio 15 2017 Win64" -DCMAKE_INSTALL_PREFIX=<INSTALL_PREFIX> -DBUILD_opencv_highgui=ON -DENABLE_FAST_MATH=ON -DBUILD_DOCS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_TESTS=OFF -DWITH_CUDA=OFF -DWITH_CUFFT=OFF -DWITH_FFMPEG=OFF -DWITH_GIGEAPI=OFF -DWITH_JASPER=OFF -DWITH_LIBV4L=OFF  -DWITH_MATLAB=OFF -DWITH_OPENCL=OFF -DWITH_OPENCLAMDBLAS=OFF -DWITH_OPENCLAMDFFT=OFF -DWITH_OPENEXR=OFF -DWITH_PVAPI=OFF -DWITH_V4L=OFF -DWITH_VTK=OFF -DWITH_WEBP=OFF -DWITH_1394=OFF -DBUILD_opencv_apps=OFF -DBUILD_opencv_calib3d=OFF -DBUILD_opencv_dnn=OFF -DBUILD_opencv_features2d=OFF -DBUILD_opencv_flann=OFF -DBUILD_opencv_ml=OFF -DBUILD_opencv_objdetect=OFF -DBUILD_opencv_shape=OFF -DBUILD_opencv_stitching=OFF -DBUILD_opencv_superres=OFF -DBUILD_opencv_ts=OFF -DBUILD_opencv_video=OFF -DBUILD_opencv_videoio=ON -DBUILD_opencv_videostab=OFF -DBUILD_opencv_world=OFF -DBUILD_opencv_highgui=ON -DPYTHON2_EXECUTABLE="" -DPYTHON3_EXECUTABLE="" -DBUILD_opencv_python2=OFF -DBUILD_opencv_python3=OFF -DWITH_QUICKTIME=OFF -DHAVE_QTKIT=FALSE
```
Then open the .sln file with Visual Studio,
check that you have "Release" and "x64" for the "Solution configurations" and "Solution platforms" (comboboxes in the toolbar),
then go to the menu "Build" and do "Build solution".  
To install OpenCV in the target directory (<INSTALL_PREFIX>), in the "Solution explorer" (on the right), right click on "INSTALL" in CMakeTargets and choose "Build".
It will install OpenCV in <INSTALL_PREFIX>, and in particular .lib files and OpenCVConfig.cmake should be in <INSTALL_PREFIX>/x64/vc15/lib and .dll files should be in <INSTALL_PREFIX>/x64/vc15/bin.

  ##### With ninja

CMake can generate a ninja build. The whole compilation is done from the command line.

First, open a "Native Tools Command Prompt" ("x64 Native Tools Command Prompt for VS 2017" for VS17 on x86-64 for example).  
`cd <OpenCV_SOURCES_DIRECTORY>`  
`mkdir build`  
`cd build`  
```
cmake <PATH_TO_CMakeLists.txt> -G "Ninja" -DCMAKE_INSTALL_PREFIX=<INSTALL_PREFIX> -DBUILD_opencv_highgui=ON -DENABLE_FAST_MATH=ON -DBUILD_DOCS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_TESTS=OFF -DWITH_CUDA=OFF -DWITH_CUFFT=OFF -DWITH_FFMPEG=OFF -DWITH_GIGEAPI=OFF -DWITH_JASPER=OFF -DWITH_LIBV4L=OFF  -DWITH_MATLAB=OFF -DWITH_OPENCL=OFF -DWITH_OPENCLAMDBLAS=OFF -DWITH_OPENCLAMDFFT=OFF -DWITH_OPENEXR=OFF -DWITH_PVAPI=OFF -DWITH_V4L=OFF -DWITH_VTK=OFF -DWITH_WEBP=OFF -DWITH_1394=OFF -DBUILD_opencv_apps=OFF -DBUILD_opencv_calib3d=OFF -DBUILD_opencv_dnn=OFF -DBUILD_opencv_features2d=OFF -DBUILD_opencv_flann=OFF -DBUILD_opencv_ml=OFF -DBUILD_opencv_objdetect=OFF -DBUILD_opencv_shape=OFF -DBUILD_opencv_stitching=OFF -DBUILD_opencv_superres=OFF -DBUILD_opencv_ts=OFF -DBUILD_opencv_video=OFF -DBUILD_opencv_videoio=ON -DBUILD_opencv_videostab=OFF -DBUILD_opencv_world=OFF -DBUILD_opencv_highgui=ON -DPYTHON2_EXECUTABLE="" -DPYTHON3_EXECUTABLE="" -DBUILD_opencv_python2=OFF -DBUILD_opencv_python3=OFF -DWITH_QUICKTIME=OFF -DHAVE_QTKIT=FALSE
```
then build with:  
`ninja`  
and install with:  
`ninja install`  

You then have to set the **OPENCV_DIR** environment variable for OpenCV to be found by CMake.   
You can open a cmd prompt "CMD.exe" with Administrator privileges (right click), and do:  
`setx -m OPENCV_DIR <INSTALL_PREFIX>`  


## Compilation

CMake is used for configuration & compilation.
The following commands allow to build DocCreator.

### Linux / macOS

On Linux and Mac, type the following commands from a terminal:  
`cd <DocCreatorDirectory>`  
`mkdir build`  
`cd build`  
`cmake .. -DCMAKE_INSTALL_PREFIX=<MyInstallationPrefix>`  
`make`  
`make install`  
You can also pass other options to cmake. See below.  

You can then launch the executable:  
`<MyInstallationPrefix>/bin/DocCreator`  

### Windows 

On Windows, with Visual Studio:  
First, open a "Developer Command Prompt" ("Developer Command Prompt for VS 2017" for VS17 for example).
(you can check that OPENCV_DIR is correctly set by doing: `echo %OPENCV_DIR%`)
go to the source directory,  
`cd <DocCreatorDirectory>`  
`mkdir build`  
`cd build`  

 #### With IDE 

CMake can generate a .sln solution file to open with Visual Studio.
 
For Visual Studio 2017: 
```
cmake .. -G "Visual Studio 15 2017 Win64" -DCMAKE_PREFIX_PATH=<QT_PATH> -DCMAKE_INSTALL_PREFIX=<MyInstallationPrefix>
```
or for Visual Studio 2019 (with CMake 3.14.6 or above, in "Developer Command Prompt for VS 2019"):
```
cmake .. -G "Visual Studio 16 2019" -A "x64" -DCMAKE_PREFIX_PATH=<QT_PATH> -DCMAKE_INSTALL_PREFIX=<MyInstallationPrefix>
```
or for Visual Studio 2022 (with CMake 3.21 or above, in "Developer Command Prompt for VS 2022"):
```
cmake .. -G "Visual Studio 17 2022" -A "x64" -DCMAKE_PREFIX_PATH=<QT_PATH> -DCMAKE_INSTALL_PREFIX=<MyInstallationPrefix>
```
<QT_PATH> is the path to installed Qt configuration for this compiler. <QT_PATH> is for example `C:/Qt/5.12.4/msvc2017_64`.
You can also pass other options to cmake. See below.  

Then you can open the generated .sln file with Visual Studio.
Check that you have "Release" and "x64" for the "Solution configurations" and "Solution platforms" (comboboxes in the toolbar),
then go to the menu "Build" and do "Build solution".
To install DocCreator in the target directory (<MyInstallationPrefix>), in the "Solution explorer" (on the right), right click on "INSTALL" in CMakeTargets and choose "Build".

  #### With Ninja

CMake can generate a ninja build. The whole compilation is done from the command line.
```
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=<QT_PATH> -DCMAKE_INSTALL_PREFIX=<MyInstallationPrefix>
```
You can also pass other options to cmake. See below.  

then build with:  
`ninja`  
then install with:  
`ninja install`  

You can then launch the executable:  
`<MyInstallationPrefix>/programs/DocCreator.exe`  

### CMake options 

#### -DBUILD_ONLY_DEGRADATIONS=ON

When configuring DocCreator with cmake, you can pass the option -DBUILD_ONLY_DEGRADATIONS=ON to cmake. It will only build the degradation models library. It can be combined with -DBUILD_WITH_OSMESA=ON to have the 3D distortion model. It can also be combined with -DBUILD_OTHER_PROGS=ON and -DBUILD_OTHER_PROGS_3D=ON.

#### -DBUILD_WITH_OSMESA=ON

When configuring DocCreator with cmake, you can pass the option -DBUILD_WITH_OSMESA=ON to cmake. It will use OSMesa, through GLFW, to have OpenGL offscreeen rendering for the 3D distortion model. This way, the 3D distortion model could be used on a linux computer without any X11 server.
GLFW will be downloaded if this option is enabled. OSMesa library must already be installed.
On Ubuntu (16.04 and above), you can install OSMesa with the following command:
`sudo apt-get install libosmesa6-dev`  
On Fedora (30 and above), you can install OSMesa with the following command as root:
`dnf install mesa-libOSMesa-devel`

#### OSMesa from sources

 OSMesa is part of Mesa.

 To build Mesa from sources, python 3.5+, Meson, mako, and ninja are required.

 ##### On macOS

 Only python 2.7 is available on macOS. You can install python 3.5+ (for example 3.8.5) with "macOS 64-bit installer" available from https://www.python.org/downloads/mac-osx/

 You should then be able to install meson and mako packages locally with pip3:
  `pip3 install --user meson`
  `pip3 install --user mako`

 You may have to add python bin path to your PATH:
 `export PATH=/Users/mansencal/Library/Python/3.8/bin:$PATH`

 As meson uses ninja, you should also install ninja (see Ninja section).

 With these dependencies installed, you should be now able to compile Mesa.
 First, get (latest) mesa archive from https://archive.mesa3d.org/
 For example mesa-20.1.7.tar.xz
 ```
tar xvJf mesa-20.1.7.tar.xz
cd mesa-20.1.7
mkdir build
 ```

 On macOS 10.15 Catalina, you may first have to edit the meson.build file: remove `'timespec_get', ` from the foreach command (line 1168).

 You should be able to configure with meson:
 ```
 meson build -Dosmesa=gallium -Dgallium-drivers=swrast -Ddri-drivers=[] -Dvulkan-drivers=[] -Dprefix=$PWD/build/install -Dplatforms=surfaceless -Dglx=disabled
 ```
 and build with ninja: 
  ```
ninja -C build install
 ```

 When configuring DocCreator, you may have to pass the OSMESA location:
 ```
cmake .. -DBUILD_WITH_OSMESA=ON -DOSMESA_INCLUDE_DIR=<mesa_directory>/build/install/include -DOSMESA_LIBRARY=<mesa_directory>/build/install/lib/libOSMesa.dylib
 ```
 Besides, if osmesa library is not installed in a standard path, you may have to add it to DYLD_LIBRARY_PATH:
 ```
 export DYLD_LIBRARY_PATH=<mesa_directory>/build/install/lib:$DYLD_LIBRARY_PATH
 ```



#### -DBUILD_OTHER_PROGS=ON

When configuring DocCreator with cmake, you can pass the option -DBUILD_OTHER_PROGS=ON to cmake. It will be build other example programs using DocCreator framework.  
In particular, it will build:  
	* **DocCreatorDegradator** that allows to apply degradation effects on all the images of a given directory and save produced images in a new directory. You can change the applied degradation effects in *tools/degradator.cpp*  
	* **DocCreatorMakeFont** that allows to create DocCreator *old-fonts* from existing (TrueType) fonts. You can save the produced files in *data/font* to use these fonts in DocCreator.  

#### -DBUILD_OTHER_PROGS_3D=ON

When configuring DocCreator with cmake, you can pass the option -DBUILD_OTHER_PROGS_3D=ON to cmake. It will be build other example programs using DocCreator 3D code.

#### -DBUILD_TESTING=ON

When configuring DocCreator with cmake, you can pass the option -DBUILD_TESTING=ON to cmake. It will be build degradations unit tests.

## FAQ 

 See [FAQ.md](FAQ.md).

## Troubleshooting


* If you encounter a problem with Qt during the cmake process, ensure that the qmake executable is in your PATH
  You maye have to do:  
  `export PATH=/Users/XXX/Qt/5.6/clang_64/bin:$PATH`  
  or  
  `export PATH=/usr/local/opt/qt5/bin:$PATH`  
  before executing `cmake ..`

* If you encounter a problem with OpenCV during the cmake process, you may have to specify the directory containing the file OpenCVConfig.cmake
 For example, on linux and macOS:    
 `OpenCV_DIR=/Users/XXX/tools/share/OpenCV cmake ..`
 or 
 `OpenCV_DIR=/Users/XXX/tools/share/opencv4 cmake ..`  
 On Windows, to set the OpenCV_DIR environment variable, you may open a cmd prompt "CMD.exe" with Administrator privileges (right click), and do:  
 `setx -m OPENCV_DIR <INSTALL_PREFIX>`  
 You will have to open a new terminal. You can print and check that the variable is correctly set with:  
 `echo %OpenCV_DIR%`  

* On Windows, if the compiler is not correctly found by cmake   
  you may have more informations in the file build/CMakeFiles/CMakeError.log  
  In particular, if ucrtd.lib is not found, you may have to install "Windows SDK".  
  Last version of Windows 10 SDK should be available from https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk
 (tested with version 10.0.15063.468).

* If CMake prints the following error:  
  "Found OpenCV Windows Pack but it has no binaries compatible with your configuration."  
  it may indicate that you have a mismatch between the architectures of OpenCV libs and your compiler.  
  For example, OpenCV libs may have been build for 64-bit architecture but your are trying to compile for 32-bit.
  On Windows with Visual Studio in particular, if you use a "Native Tools Command Prompt", check that it is for the correct architecture. For example "x64 Native Command Prompt" and not "x86 Native Command Prompt" for x86_64. 

* If CMake fails to download one of the dependencies and prints this kind of error:  
```
  error: downloading
  https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.tar.gz
  failed: status code=1 string="Unsupported protocol", log=Protocol "https"
  not supported or disabled in libcurl
```
 the problem seems to be that the CURL library shipped with CMake is not build with SSL support.
 You may have to compile CMake from sources enabling using a CURL library with SSL support.

* If you encounter a problem during the make process, it may be related to the C++11 detection
  edit CMakeLists.txt and add (or remove) the following line before the first "AD_SUBDIRECTORY" line  
  `SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")`  
  then restart the construction process:  
  `rm -rf CMakeCache.txt CMakeFiles`  
  `cmake ..`  
  `make`  

* On Windows, with Visual Studio, if you encounter a link problem with tesseract, it may be that you compiled in Debug mode instead of Release mode.
  Indeed, DocCreator tries to link with tesseract305.lib, but tesseract305d.lib is generated in Debug mode. 

* On Windows, if you encounter a problem when the program is launched, it may be that the various required libraries (dll) are not correctly found.  
You then may have to copy some files to <MyInstallationPrefix>/programs directory [if they are not present]:
- from OpenCV <INSTALL_PREFIX>/x64/vc15/bin :  
  opencv_coreXXX.dll, opencv_highguiXXX.dll, opencv_imgcodecsXXX.dll, opencv_imgprocXXX.dll, opencv_photoXXX.dll, opencv_videoioXXX.dll (XXX has to be replaced by 330 for OpenCV 3.3)
- from Qt:  
  - from <QT_PATH>/bin :  
     Qt5Core.dll, Qt5Gui.dll, Qt5Network.dll, Qt5OpenGL.dll, qt5PrintSupport.dll, Qt5Widgets.dll, Qt5Xml.dll, Qt5XmlPatterns.dll,
	 libEGL.dll, libGLESv2.dll, d3dcompiler_47.dll
  - from <QT_PATH>/plugins :  
     *imageformats* directory
     *platforms* directory


