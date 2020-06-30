cmake_minimum_required(VERSION 3.5)
project(boxfilter)

set(CMAKE_CXX_STANDARD 11)


# Only usefull in gcc
if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif ()

add_definitions(-fPIC)
add_definitions(-Ofast)
add_definitions(-ffast-math)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fvisibility=hidden -fvisibility-inlines-hidden")

add_subdirectory(src)

add_subdirectory(3rdparty/googletest/googletest)
add_subdirectory(unit_test)