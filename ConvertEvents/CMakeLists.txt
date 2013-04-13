PROJECT(ConvertEvents)

INCLUDE_DIRECTORIES(
	${edvstools_SOURCE_DIR}
)

ADD_EXECUTABLE(${PROJECT_NAME}
	main.cpp
	LoadSaveEvents.cpp
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	Edvs
	boost_program_options
	boost_filesystem
	boost_system
)
