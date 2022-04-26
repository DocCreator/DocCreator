PROJECT(lzhamcomp)
cmake_minimum_required(VERSION 2.8.12)

# Honor visibility properties for all target types. [Policy introduced in cmake 3.3]
IF(POLICY CMP0063)
  CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

option(BUILD_X64 "build 64-bit" TRUE)

#message("Initial BUILD_X64=${BUILD_X64}")
#message("Initial CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release )
endif( NOT CMAKE_BUILD_TYPE )

#message( ${PROJECT_NAME} " build type: " ${CMAKE_BUILD_TYPE} )

#if (BUILD_X64)
#	message("Building 64-bit")
#else()
#	message("Building 32-bit")
#endif(BUILD_X64)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${WARNING_FLAGS} ${DEBUG_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${WARNING_FLAGS} ${DEBUG_FLAGS}")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${WARNING_FLAGS} ${RELEASE_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${WARNING_FLAGS} ${RELEASE_FLAGS}")

set(SRC_LIST 
	lzham_lzbase.cpp
	lzham_lzbase.h
	lzham_lzcomp.cpp
	lzham_lzcomp_internal.cpp
	lzham_lzcomp_internal.h
	lzham_lzcomp_state.cpp
	lzham_match_accel.cpp
	lzham_match_accel.h
	lzham_null_threading.h
	lzham_pthreads_threading.cpp
	lzham_pthreads_threading.h
	lzham_threading.h)

if (WIN32)
  list (APPEND SRC_LIST lzham_win32_threading.cpp)
endif ()

# -fno-strict-aliasing is *required* to compile LZHAM
if (NOT MSVC)
	set(GCC_COMPILE_FLAGS "-fno-strict-aliasing -D_LARGEFILE64_SOURCE=1 -D_FILE_OFFSET_BITS=64")
endif()

if (NOT BUILD_X64)
	set(GCC_COMPILE_FLAGS "${GCC_COMPILE_FLAGS} -m32")
endif()

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_COMPILE_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE  "${CMAKE_C_FLAGS_RELEASE} ${GCC_COMPILE_FLAGS} -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${GCC_COMPILE_FLAGS} -D_DEBUG")

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} ${GCC_COMPILE_FLAGS} -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${GCC_COMPILE_FLAGS} -D_DEBUG")

include_directories(
	${PROJECT_SOURCE_DIR}/../lzhamdecomp 
	${PROJECT_SOURCE_DIR}/../include)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin_linux)

add_library(${PROJECT_NAME} ${SRC_LIST})
if(NOT MSVC)
	set(THREADLIB pthread)
endif(NOT MSVC)
target_link_libraries(${PROJECT_NAME} lzhamdecomp ${THREADLIB})
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib${LIB_SUFFIX} ARCHIVE DESTINATION lib${LIB_SUFFIX} RUNTIME DESTINATION bin)
