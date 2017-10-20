#-------------------------------------------------
#
# Project created by QtCreator 2010-05-10T18:09:07
#
#-------------------------------------------------

QT       += core network xml
QT       -= gui

TARGET = Lipsum4Qt
TEMPLATE = lib

macx{
 CONFIG += lib_bundle
 FRAMEWORK_HEADERS.version = Versions
 FRAMEWORK_HEADERS.files = Lipsum4Qt.hpp\
                        Lipsum4Qt_global.h
 FRAMEWORK_HEADERS.path = Headers
 QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
}

DEFINES += LIPSUM4QT_LIBRARY

SOURCES += Lipsum4Qt.cpp

HEADERS += Lipsum4Qt.hpp\
        Lipsum4Qt_global.h
