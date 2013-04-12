PROJECT(Edvs)

ADD_LIBRARY(${PROJECT_NAME} SHARED
	EventStreamFactory.cpp
	FileEventStream.cpp
	LoadSaveEvents.cpp
	RawDeviceEventStream.cpp
	RawEventTranslator.cpp
	RawFileEventStream.cpp
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	boost_filesystem
	boost_thread
)
