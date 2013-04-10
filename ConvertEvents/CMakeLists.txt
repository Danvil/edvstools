PROJECT(ConvertEvents)

include_directories(
	${EIGEN_INCLUDE_DIR}
	${EdvsTools_SOURCE_DIR}
)

ADD_EXECUTABLE(${PROJECT_NAME}
	main.cpp
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	EdvsTools
	boost_program_options
)
