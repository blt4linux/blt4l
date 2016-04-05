cmake_minimum_required(VERSION 3.0.0)
project(blt4l)

add_subdirectory(subhook)

add_definitions(-D_GNU_SOURCE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fpermissive")

include_directories(include subhook)

set(SOURCE_FILES
    src/hook.cc
    src/lapi.cc
    )

add_library(blt_loader SHARED ${SOURCE_FILES} src/blt_main.cc)

target_link_libraries(blt_loader subhook dl)
