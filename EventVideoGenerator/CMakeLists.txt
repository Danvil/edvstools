PROJECT(EventVideoGenerator)

INCLUDE_DIRECTORIES(
	${edvstools_SOURCE_DIR}
)

ADD_EXECUTABLE(${PROJECT_NAME}
	main.cpp
	lodepng.cpp
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	Edvs
	boost_program_options
)
