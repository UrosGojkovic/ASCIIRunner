QT -= gui
QT += core

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG += qt

QMAKE_CXXFLAGS += -std=c++0x
QMAKE_CXXFLAGS += -lncurses
QMAKE_LFLAGS += -lncurses

SOURCES += main.cpp \
    asciirenderengine.cpp \
    Generisi.cpp \
    asciirunner.cpp

HEADERS += \
    asciirenderengine.h \
    asciirunner.h

