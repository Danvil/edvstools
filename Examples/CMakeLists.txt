PROJECT(Examples)

INCLUDE_DIRECTORIES(${edvstools_SOURCE_DIR})

ADD_EXECUTABLE(example_capture_c example_capture.c)
SET_SOURCE_FILES_PROPERTIES(example_capture.c PROPERTIES COMPILE_FLAGS "-std=c99 -Wall")
TARGET_LINK_LIBRARIES(example_capture_c Edvs)

ADD_EXECUTABLE(example_capture_cpp example_capture.cpp)
TARGET_LINK_LIBRARIES(example_capture_cpp Edvs)
