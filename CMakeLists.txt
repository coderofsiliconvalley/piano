cmake_minimum_required(VERSION 2.8)

project(piano)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER MATCHES ".*clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -Wno-error=unused-but-set-variable -std=c++11")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmakemodules")

find_package(OpenCV REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Alure REQUIRED)

file(GLOB piano_sources *.cpp)
add_executable(piano ${piano_sources})
include_directories(${OPENAL_INCLUDE_DIR})
include_directories(${ALURE_INCLUDE_DIRS})
target_link_libraries(piano ${OpenCV_LIBS} ${OPENAL_LIBRARY} ${ALURE_LIBRARIES})



file(COPY samples DESTINATION ./bin)
file(COPY video5.mp4 DESTINATION ./bin)
