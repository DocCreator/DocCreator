FRAMEWORK = ../../framework
BUILD = ../../../build
INCLUDEPATH += $$FRAMEWORK
LIBS += -L$$BUILD
LIBS += -lframework
QT += testlib

# CONFIG  += qtestlib
SOURCES += frameworktest.cpp
HEADERS += frameworktest.h
