cmake_minimum_required(VERSION 2.8)

project(piano)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER MATCHES ".*clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -std=c++11")
endif()


find_package(OpenCV REQUIRED)

file(GLOB piano_sources *.cpp)
add_executable(piano ${piano_sources})
target_link_libraries(piano ${OpenCV_LIBS})
