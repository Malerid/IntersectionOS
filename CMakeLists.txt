cmake_minimum_required(VERSION 2.8.4)
project(intersectionOS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread")

set(SOURCE_FILES main.cpp)
add_executable(intersectionOS ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(intersectionOS pthread)