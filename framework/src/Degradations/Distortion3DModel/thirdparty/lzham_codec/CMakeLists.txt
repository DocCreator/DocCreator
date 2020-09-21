# PROJECT(lzham)
cmake_minimum_required(VERSION 2.8)
option(BUILD_X64 "build 64-bit" ON)
option(BUILD_SHARED_LIBS "build shared/static libs" ON)

if(MSVC)
	set(WARNING_FLAGS "-Wall /W4")
	set(DEBUG_FLAGS "/Zi")
	set(RELEASE_FLAGS "/O2 /Oy")
else()	
	set(WARNING_FLAGS "-Wall -Wextra")
	set(DEBUG_FLAGS "-g")
	set(RELEASE_FLAGS "-O3 -fomit-frame-pointer -fexpensive-optimizations")
endif()

add_subdirectory(lzhamdecomp)
add_subdirectory(lzhamcomp)
add_subdirectory(lzhamdll)
#add_subdirectory(lzhamtest)

install(FILES include/lzham_dynamic_lib.h
              include/lzham_exports.inc
              include/lzham.h
              include/lzham_static_lib.h
              include/zlib.h
              DESTINATION include)
