include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set (SRCS BoxFilter.cpp)
add_library(BoxFilter SHARED ${SRCS})