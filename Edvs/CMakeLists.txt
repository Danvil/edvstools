PROJECT(Edvs)

ADD_LIBRARY(${PROJECT_NAME} SHARED
	edvs.c
	EventIO.cpp
	EventStream.cpp
)

set_source_files_properties(edvs.c PROPERTIES COMPILE_FLAGS "-std=c99 -Wall -D_GNU_SOURCE")

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	boost_thread
	boost_system
	rt
)
