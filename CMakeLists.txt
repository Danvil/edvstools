cmake_minimum_required(VERSION 2.6)

add_definitions(-std=c++0x)

include_directories(
	Edvs
)

project(edvstools)

add_subdirectory(CheckEvents)
add_subdirectory(ConvertEvents)
add_subdirectory(Edvs)
add_subdirectory(EventVideoGenerator)
#add_subdirectory(EventViewer)
add_subdirectory(RecordEvents)
add_subdirectory(ShowRetina)
add_subdirectory(TestConnection)
add_subdirectory(Examples)
