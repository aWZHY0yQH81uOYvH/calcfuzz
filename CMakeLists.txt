cmake_minimum_required(VERSION 3.0)
project(calcrev)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


add_executable(calcfuzz calcfuzz.cpp)
target_link_libraries(calcfuzz ${OpenCV_LIBS})
