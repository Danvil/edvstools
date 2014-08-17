PROJECT(EventViewer)

find_package(Qt4 REQUIRED)

INCLUDE_DIRECTORIES(
	${EIGEN_INCLUDE_DIR}
	${edvstools_SOURCE_DIR}
)

SET(QT_USE_QTOPENGL TRUE)

SET(EventViewer_SOURCES
	main.cpp
	WdgtEventViewer.cpp
	WdgtCameraParameters.cpp
	../../tools/ConvertEvents/LoadSaveEvents.cpp
)
SET(EventViewer_HEADERS
	WdgtEventViewer.h
	WdgtCameraParameters.h
)
SET(EventViewer_FORMS
	WdgtEventViewer.ui
	WdgtCameraParameters.ui
)

QT4_WRAP_CPP(EventViewer_HEADERS_MOC ${EventViewer_HEADERS})
QT4_WRAP_UI(EventViewer_FORMS_HEADERS ${EventViewer_FORMS})

INCLUDE(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS} -DQT_NO_KEYWORDS)

ADD_EXECUTABLE(${PROJECT_NAME}
	${EventViewer_SOURCES}
	${EventViewer_HEADERS_MOC}
	${EventViewer_FORMS_HEADERS}
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	Edvs
	${QT_LIBRARIES}
	boost_program_options
)

