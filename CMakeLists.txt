cmake_minimum_required(VERSION 3.0.0)
project(blt4l)

add_definitions(-D_GNU_SOURCE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fpermissive --std=gnu++11")

include_directories(include)

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
    )

add_library(blt_loader SHARED ${SOURCE_FILES} src/blt_main.cc)

target_link_libraries(blt_loader dl curl ssl z)
