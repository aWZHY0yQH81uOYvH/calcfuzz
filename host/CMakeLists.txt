cmake_minimum_required(VERSION 3.0)
project(calcrev)

add_compile_options(-Wall -Wextra)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


add_executable(calcfuzz
	calcfuzz.cpp
	buttons.cpp

	calccomm.hpp
	buttons.hpp
	generator.hpp
)

target_link_libraries(calcfuzz ${OpenCV_LIBS})


add_executable(digitprocess
	digitprocess.cpp

	digit.hpp
)

target_link_libraries(digitprocess ${OpenCV_LIBS})
