PROJECT(event_video_generator)

include_directories(
	${EIGEN_INCLUDE_DIR}
	${EdvsTools_SOURCE_DIR}
)

ADD_EXECUTABLE(${PROJECT_NAME}
	event_video_generator.cpp
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	EdvsTools
	boost_program_options
)
