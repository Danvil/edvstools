cmake_minimum_required(VERSION 2.6)

set(EIGEN_INCLUDE_DIR "/usr/include/eigen3" CACHE STRING "Eigen3 include folder")

find_package(octomap REQUIRED)
include_directories(${OCTOMAP_INCLUDE_DIRS})
link_directories(${OCTOMAP_LIBRARY_DIRS})
link_libraries(${OCTOMAP_LIBRARIES})

add_definitions(-std=c++0x)

include_directories(
	libedvs
)

project(edvs)

add_subdirectory(libedvs)
add_subdirectory(EdvsTools)
add_subdirectory(EdvsVisual)

add_subdirectory(EdvsTest)
add_subdirectory(EdvsTracking)

add_subdirectory(check_events)
add_subdirectory(convert_matlab_events)
add_subdirectory(EventViewer)
add_subdirectory(event_video_generator)

add_subdirectory(SimulateMap)
add_subdirectory(SimulatePath)
add_subdirectory(SimulateEvents)

#add_subdirectory(CalibrateExternalToMap)
add_subdirectory(ScenarioPrepare)
add_subdirectory(ScenarioTransformPath)
add_subdirectory(ConvertEvents)

#add_subdirectory(CorrectCameraTilt)
add_subdirectory(EvaluateTracker)
#add_subdirectory(CreateMap)
#add_subdirectory(CannyEdge)

#add_subdirectory(ARTracking)
#add_subdirectory(ARTrackingImages)
#add_subdirectory(ARCamCalibImages)

#add_subdirectory(GlobalOptimalSolution)
add_subdirectory(PathOptimizer)
add_subdirectory(Tools)
