PROJECT(EdvsVisual)

find_package(Qt4 REQUIRED)

include_directories(
	src
)

SET(QT_USE_QTOPENGL TRUE)

SET(edvs_visual_SOURCES
	main.cpp
	WdgtEdvsVisual.cpp
)
SET(edvs_visual_HEADERS
	WdgtEdvsVisual.h
)
SET(edvs_visual_FORMS
	WdgtEdvsVisual.ui
)

QT4_WRAP_CPP(edvs_visual_HEADERS_MOC ${edvs_visual_HEADERS})
QT4_WRAP_UI(edvs_visual_FORMS_HEADERS ${edvs_visual_FORMS})

INCLUDE(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS} -DQT_NO_KEYWORDS)

ADD_EXECUTABLE(EdvsVisual
	${edvs_visual_SOURCES}
	${edvs_visual_HEADERS_MOC}
	${edvs_visual_FORMS_HEADERS}
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

TARGET_LINK_LIBRARIES(EdvsVisual
	${QT_LIBRARIES}
	boost_program_options
	boost_signals
	boost_thread
)

