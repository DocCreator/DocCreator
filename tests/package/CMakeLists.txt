# A project outside the DocCreator tree, that uses the installed degradation
# library through its CMake package only. testInstalledPackage installs the
# library, then configures, builds and runs this project against it (cf.
# tests/CMakeLists.txt and RunPackageTest.cmake).

cmake_minimum_required(VERSION 3.16)
project(DocCreatorPackageConsumer CXX)

find_package(DocCreator 0 REQUIRED COMPONENTS Degradations)

add_executable(consumer consumer.cpp)
target_link_libraries(consumer PRIVATE DocCreator::Degradations)
