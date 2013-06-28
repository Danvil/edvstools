PROJECT(crossing-detector)

find_package(Qt4 REQUIRED)

INCLUDE_DIRECTORIES(
	${edvstools_SOURCE_DIR}
)

SET(QT_USE_QTOPENGL TRUE)

SET(crossdetect_SOURCES
	main.cpp
	WdgtEdvsVisual.cpp
)
SET(crossdetect_HEADERS
	WdgtEdvsVisual.h
)
SET(crossdetect_FORMS
	WdgtEdvsVisual.ui
)

QT4_WRAP_CPP(crossdetect_HEADERS_MOC ${crossdetect_HEADERS})
QT4_WRAP_UI(crossdetect_FORMS_HEADERS ${crossdetect_FORMS})

INCLUDE(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS} -DQT_NO_KEYWORDS)

ADD_EXECUTABLE(${PROJECT_NAME}
	${crossdetect_SOURCES}
	${crossdetect_HEADERS_MOC}
	${crossdetect_FORMS_HEADERS}
)

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_BINARY_DIR}
	${EIGEN_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	Edvs
	${QT_LIBRARIES}
	boost_program_options
	boost_signals
)

