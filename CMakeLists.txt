cmake_minimum_required(VERSION 2.6)

set(EIGEN_INCLUDE_DIR "/usr/include/eigen3" CACHE STRING "Eigen3 include folder")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")

add_definitions(-std=c++0x)

include_directories(
	Edvs
)

project(edvstools)

add_subdirectory(Edvs)

add_subdirectory(tools/ConvertEvents)
add_subdirectory(tools/EventVideoGenerator)
add_subdirectory(tools/ShowEvents)

add_subdirectory(aux/TestConnection)
add_subdirectory(aux/CheckEvents)
add_subdirectory(aux/Terminal)
add_subdirectory(aux/Examples)
add_subdirectory(aux/EventViewer)

add_subdirectory(aux/MathLink)
