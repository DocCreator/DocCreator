
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

FIND_PACKAGE(Qt5Widgets)
FIND_PACKAGE(Qt5OpenGL)

# Qt5 and all the required modules are present, do global setup
IF(${Qt5Widgets_FOUND} AND ${Qt5OpenGL_FOUND})

    #With QT5.0 & QT5.1 it seems that QT_PLUGINS_DIR is not longer set.
    #It should be fixed with Qt5.2
    # http://stackoverflow.com/questions/14183441/path-for-qt5-plugins-in-a-cmake-managed-project
    # https://codereview.qt-project.org/#patch,all_unified,63100,9
    GET_TARGET_PROPERTY(Qt5Core_location Qt5::Core LOCATION)
    GET_FILENAME_COMPONENT(QT_BINARY_DIR ${Qt5Core_location} PATH)
    IF(WIN32)
        SET(QT_BINARY_DIR ${QT_BINARY_DIR}/../bin)
        SET(QT_PLUGINS_DIR ${QT_BINARY_DIR}/../plugins)
    ENDIF(WIN32)
    IF(UNIX)
        SET(QT_PLUGINS_DIR ${QT_BINARY_DIR}/qt5/plugins) #specific Fedora ???	
    ENDIF(UNIX)

    SET(QT_LIBRARIES ${Qt5Widgets_LIBRARIES} ${Qt5OpenGL_LIBRARIES})

    #Needed to compile an executable !
    #All variables Qt5Core|Widgets|Gui|OpenGL_EXECUTABLE_COMPILE_FLAGS seems to be set to the same thing (-fPIE).
    SET(QTX_EXECUTABLE_COMPILE_FLAGS ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS})

ENDIF(${Qt5Widgets_FOUND} AND ${Qt5OpenGL_FOUND})
