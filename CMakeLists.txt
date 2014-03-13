cmake_minimum_required(VERSION 2.6)

set(EIGEN_INCLUDE_DIR "/usr/include/eigen3" CACHE STRING "Eigen3 include folder")

add_definitions(-std=c++0x)

include_directories(
	Edvs
)

project(edvstools)

add_subdirectory(Edvs)

add_subdirectory(TestConnection)
add_subdirectory(ShowEvents)

add_subdirectory(RecordEvents)
add_subdirectory(CheckEvents)
add_subdirectory(ConvertEvents)
add_subdirectory(EventVideoGenerator)
add_subdirectory(EventViewer)
add_subdirectory(Terminal)

add_subdirectory(MathLink)

add_subdirectory(MultiEdvs)

add_subdirectory(Examples)
