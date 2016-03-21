cmake_minimum_required(VERSION 3.4.1)
project(blt4l)

add_definitions(-D_GNU_SOURCE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ldl -lsubhook -fPIC -fpermissive")

include_directories(include)

set(SOURCE_FILES
    src/hook.cc
    )

add_library(blt_loader SHARED ${SOURCE_FILES} src/blt_main.cc)
