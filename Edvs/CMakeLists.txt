PROJECT(Edvs)

ADD_LIBRARY(${PROJECT_NAME} SHARED
	DeviceEventStream.cpp
	EventCapture.cpp
	EventStreamFactory.cpp
	LoadSaveEvents.cpp
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	boost_filesystem
	boost_thread
)
