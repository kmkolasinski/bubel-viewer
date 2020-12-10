VPATH += ../shared
INCLUDEPATH += ../shared

HEADERS       = glwidget.h \
                window.h \
    datareader.h \
    mainwindow.h \
    formleadparams.h \
    camera.h
SOURCES       = glwidget.cpp \
                main.cpp \
                window.cpp \
    datareader.cpp \
    mainwindow.cpp \
    formleadparams.cpp \
    camera.cpp
QT           += opengl widgets
QT           += xml
LIBS += -lGLU
# install
INSTALLS += target sources

FORMS += \
    mainwindow.ui \
    formleadparams.ui

RESOURCES += \
    resources.qrc

ICON = icons.png

