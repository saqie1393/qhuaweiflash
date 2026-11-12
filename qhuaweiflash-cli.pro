
TEMPLATE = app
TARGET = qhuaweiflash-cli

QT += widgets
CONFIG += console c++11
CONFIG -= app_bundle

INCLUDEPATH += .

SOURCES += \
    cli_main.cpp \
    ptable.cpp \
    signver.cpp

HEADERS += \
    ptable.h \
    signver.h

LIBS += -lz
