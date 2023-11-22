cmake_minimum_required(VERSION 3.0)
project(calcrev)

add_compile_options(-Wall -Wextra)

find_package(OpenCV REQUIRED COMPONENTS
	core
	video
	videoio
	highgui
	imgproc
)

include_directories(${OpenCV_INCLUDE_DIRS})


add_executable(calcfuzz
	calcfuzz.cpp
	Buttons.cpp

	Calculator.hpp
	Buttons.hpp
	Generator.hpp
)

target_link_libraries(calcfuzz ${OpenCV_LIBS})


add_executable(digitprocess
	digitprocess.cpp

	Digit.hpp
)

target_link_libraries(digitprocess ${OpenCV_LIBS})
