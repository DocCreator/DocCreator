###inspired from FindQTX.cmake from tulip software.

SET(USE_QT5_IF_INSTALLED ON CACHE BOOL "Use Qt5 to build the project if it is installed.")

SET(Qt5Widgets_FOUND false)
SET(Qt5Xml_FOUND false)
SET(Qt5XmlPatterns_FOUND false)
SET(Qt5PrintSupport_FOUND false)
SET(Qt5OpenGL_FOUND false)
SET(Qt5Network_FOUND false)

# If asked, use Qt5
IF(USE_QT5_IF_INSTALLED)

    FIND_PACKAGE(Qt5Widgets)
    FIND_PACKAGE(Qt5Xml)
    FIND_PACKAGE(Qt5XmlPatterns)
    FIND_PACKAGE(Qt5PrintSupport)
    FIND_PACKAGE(Qt5OpenGL)
    FIND_PACKAGE(Qt5Network)

ENDIF(USE_QT5_IF_INSTALLED)


function(stripLibName name result)
    # We have for example name=/usr/lib64/libQt5Core.so.5.1.1
    # we would like to remove the ".5.1.1" part

    # OLD :
    # We do this with STRING(REGEX MATCH "^[^\\.]+\\.[^\\.]+" result Var)
    # that matches string that starts with everything execept a dot, followed by a dot, followed by everything except a dot
    # Warning: it will not work if Var lists several libraries !
    #STRING(REGEX MATCH "^[^\\.]+\\.[^\\.]+" resultTmp "${name}")
    # It fails if we have for example a path like : /aaaa/bbbb/QT5.2.0/libQt5Widgets.so.5.2.0

    # We do this with STRING(REGEX MATCH ".*\\.(so|dylib)" result Var)
    # that matches string that stats with anything and end with ".so" or ".dylib"
    STRING(REGEX MATCH ".*\\.(so|dylib)" resultTmp "${name}")

    SET(${result} ${resultTmp} PARENT_SCOPE)
    #MESSAGE("name=${name}")
    #MESSAGE("resultTmp=${resultTmp}")
endfunction()


# Qt5 and all the required modules are present, do global setup
IF(${Qt5Widgets_FOUND} AND ${Qt5Xml_FOUND} AND ${Qt5XmlPatterns_FOUND} AND ${Qt5OpenGL_FOUND} AND ${Qt5Network_FOUND})

    SET(USE_QT5 true)

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

#     INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDE_DIRS})
#     INCLUDE_DIRECTORIES(${Qt5Xml_INCLUDE_DIRS})
#     INCLUDE_DIRECTORIES(${Qt5XmlPatterns_INCLUDE_DIRS})
#     INCLUDE_DIRECTORIES(${Qt5PrintSupport_INCLUDE_DIRS})
#     INCLUDE_DIRECTORIES(${Qt5OpenGL_INCLUDE_DIRS})
#     INCLUDE_DIRECTORIES(${Qt5Network_INCLUDE_DIRS})

#     ADD_DEFINITIONS(${Qt5Widgets_DEFINITIONS})
#     ADD_DEFINITIONS(${Qt5Xml_DEFINITIONS})
#     ADD_DEFINITIONS(${Qt5XmlPatterns_DEFINITIONS})
#     ADD_DEFINITIONS(${Qt5PrintSupport_DEFINITIONS})
#     ADD_DEFINITIONS(${Qt5OpenGL_DEFINITIONS})
#     ADD_DEFINITIONS(${Qt5Network_DEFINITIONS})

    SET(QT_LIBRARIES ${Qt5Widgets_LIBRARIES} ${Qt5Xml_LIBRARIES} ${Qt5XmlPatterns_LIBRARIES} ${Qt5PrintSupport_LIBRARIES} ${Qt5OpenGL_LIBRARIES} ${Qt5Network_LIBRARIES})

    #We want the variables that were defined for Qt4 :
    # QT_QTCORE_LIBRARY_DEBUG, QT_QTGUI_LIBRARY_DEBUG, QT_QTOPENGL_LIBRARY_DEBUG
    # QT_QTCORE_LIBRARY_RELEASE, QT_QTGUI_LIBRARY_RELEASE, QT_QTOPENGL_LIBRARY_RELEASE
    #
    # ${Qt5Core_LIBRARIES} is Qt5::Core : it seems enough for CMake to link, 
    # but this is not what we want for the .pc file.
    #   
    # ${Qt5Core_location} is /usr/lib64/libQt5Core.so.5.1.1
    # we would like to remove the ".5.1.1" part
    # Warning: it will not work if Var lists several libraries !
    
    GET_TARGET_PROPERTY(Qt5Gui_location Qt5::Gui LOCATION)
    GET_TARGET_PROPERTY(Qt5Widgets_location Qt5::Widgets LOCATION)
    GET_TARGET_PROPERTY(Qt5Xml_location Qt5::Xml LOCATION)
    GET_TARGET_PROPERTY(Qt5XmlPatterns_location Qt5::XmlPatterns LOCATION)
    GET_TARGET_PROPERTY(Qt5PrintSupport_location Qt5::PrintSupport LOCATION)
    GET_TARGET_PROPERTY(Qt5OpenGL_location Qt5::OpenGL LOCATION)
    GET_TARGET_PROPERTY(Qt5Network_location Qt5::Network LOCATION)

    stripLibName("${Qt5Core_location}" QT_QTCORE_LIBRARY)
    set(QT_QTCORE_LIBRARY_DEBUG ${QT_QTCORE_LIBRARY})
    set(QT_QTCORE_LIBRARY_RELEASE ${QT_QTCORE_LIBRARY})

    stripLibName("${Qt5Gui_location}" QT_QTGUI_LIBRARY)
    stripLibName("${Qt5Widgets_location}" QT_QTGUI_LIBRARY2)
    set(QT_QTGUI_LIBRARY_DEBUG ${QT_QTGUI_LIBRARY} ${QT_QTGUI_LIBRARY2})
    set(QT_QTGUI_LIBRARY_RELEASE ${QT_QTGUI_LIBRARY} ${QT_QTGUI_LIBRARY2})

    stripLibName("${Qt5Xml_location}" QT_QTXML_LIBRARY)
    set(QT_QTXML_LIBRARY_DEBUG ${QT_QTXML_LIBRARY})
    set(QT_QTXML_LIBRARY_RELEASE ${QT_QTXML_LIBRARY})

    stripLibName("${Qt5XmlPatterns_location}" QT_QTXMLPATTERNS_LIBRARY)
    set(QT_QTXMLPATTERNS_LIBRARY_DEBUG ${QT_QTXMLPATTERNS_LIBRARY})
    set(QT_QTXMLPATTERNS_LIBRARY_RELEASE ${QT_QTXMLPATTERNS_LIBRARY})

    stripLibName("${Qt5PrintSupport_location}" QT_QTPRINTSUPPORT_LIBRARY)
    set(QT_QTPRINTSUPPORT_LIBRARY_DEBUG ${QT_QTPRINTSUPPORT_LIBRARY})
    set(QT_QTPRINTSUPPORT_LIBRARY_RELEASE ${QT_QTPRINTSUPPORT_LIBRARY})

    stripLibName("${Qt5OpenGL_location}" QT_QTOPENGL_LIBRARY)
    set(QT_QTOPENGL_LIBRARY_DEBUG ${QT_QTOPENGL_LIBRARY})
    set(QT_QTOPENGL_LIBRARY_RELEASE ${QT_QTOPENGL_LIBRARY})

    stripLibName("${Qt5Network_location}" QT_QTNETWORK_LIBRARY)
    set(QT_QTNETWORK_LIBRARY_DEBUG ${QT_QTNETWORK_LIBRARY})
    set(QT_QTNETWORK_LIBRARY_RELEASE ${QT_QTNETWORK_LIBRARY})


    # How to get different debug/release libs ???

    #Needed to compile an executable !
    #All variables Qt5Core|Widgets|Gui|Xml_EXECUTABLE_COMPILE_FLAGS seems to be set to the same thing (-fPIE).
    SET(QTX_EXECUTABLE_COMPILE_FLAGS ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS})

    # define aliases for Qt macros in order to build the project
    MACRO(QTX_WRAP_CPP outfiles )
      QT5_WRAP_CPP(${outfiles} ${ARGN})
    ENDMACRO()

    MACRO(QTX_WRAP_UI outfiles )
      QT5_WRAP_UI(${outfiles} ${ARGN})
    ENDMACRO()

    MACRO(QTX_ADD_RESOURCES outfiles )
      QT5_ADD_RESOURCES(${outfiles} ${ARGN})
    ENDMACRO()

# Use Qt4 otherwise
ELSE(${Qt5Widgets_FOUND} AND ${Qt5Xml_FOUND} AND ${Qt5XmlPatterns_FOUND} AND ${Qt5OpenGL_FOUND} AND ${Qt5Network_FOUND})

    IF(USE_QT5_IF_INSTALLED)
      MESSAGE("Qt 5 required components or the CMake modules to locate them have not been found.")
      MESSAGE("Falling back to Qt 4.")
    ENDIF(USE_QT5_IF_INSTALLED)

    FIND_PACKAGE(Qt4 4.2.0 REQUIRED)

    SET(USE_QT4 true)
    SET(QT_USE_QTOPENGL true)
    SET(QT_USE_QTNETWORK true)
    SET(QT_USE_QTXML true)
    SET(QT_USE_QTTEST false)
    #SET(QT_USE_QTDBUS false)
    INCLUDE(${QT_USE_FILE})


    SET(QTX_EXECUTABLE_COMPILE_FLAGS "")

     # define aliases for Qt macros
    MACRO(QTX_WRAP_CPP outfiles )
      QT4_WRAP_CPP(${outfiles} ${ARGN})
    ENDMACRO()

    MACRO(QTX_WRAP_UI outfiles )
      QT4_WRAP_UI(${outfiles} ${ARGN})
    ENDMACRO()

    MACRO(QTX_ADD_RESOURCES outfiles )
      QT4_ADD_RESOURCES(${outfiles} ${ARGN})
    ENDMACRO()

ENDIF(${Qt5Widgets_FOUND} AND ${Qt5Xml_FOUND} AND ${Qt5XmlPatterns_FOUND}  AND ${Qt5OpenGL_FOUND} AND ${Qt5Network_FOUND})
