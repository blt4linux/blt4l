cmake_minimum_required(VERSION 2.8.0)

# You can do it with GCC, but it's complicated
option(USE_LIBCXX "build BLT with libc++ instead of libstdc++, requires clang" OFF)

# LibCXX / Patches support
if (USE_LIBCXX)
   message("Using Clang/LibC++")
   add_definitions(-DBLT_USING_LIBCXX)
   set(CMAKE_CXX_COMPILER clang++)
endif()

project(blt4l)

find_package(CURL)
find_package(OpenSSL)
find_package(ZLIB)

if(NOT (CURL_FOUND AND OPENSSL_FOUND AND ZLIB_FOUND))
	message(FATAL_ERROR "One of the needed libraries wasn't found! Exiting")
endif()

add_definitions(-D_GNU_SOURCE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11")
if (USE_LIBCXX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

include_directories(include subhook)

set(SOURCE_FILES
    subhook/subhook.c

    src/event.cc
    src/zip.cc
    src/fs.cc
    src/http.cc
    src/log.cc
    src/hook.cc
    src/lapi.cc
    src/lapi_http.cc
    src/lapi_version.cc
    src/lapi_systemfs.cc

    src/patches/workshop.cc
    )

add_library(blt_loader SHARED ${SOURCE_FILES} src/blt_main.cc)

target_link_libraries(blt_loader
	dl
    crypto
	${CURL_LIBRARIES}
	${OpenSSL_LIBRARIES}
	${ZLIB_LIBRARIES}
	)
