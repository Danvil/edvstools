TEMPLATE = app
QT += core \
    gui
GIT_BASE = /home/david/git
INCLUDEPATH += $$GIT_BASE/Edvs/eDVS128
HEADERS += WdgtEdvsVisual.h
SOURCES += WdgtEdvsVisual.cpp \
    main.cpp
FORMS += WdgtEdvsVisual.ui
RESOURCES += 
CONFIG(debug, debug|release) { 
    DEFINES += DEBUG
    TARGET = EdvsVisual-Debug
    LIBS += 
}
CONFIG(release, debug|release) { 
    DEFINES += NDEBUG
    TARGET = EdvsVisual
    LIBS += 
}
LIBS += -lboost_thread
