

SET(
  headers_context
  src/context/appcontext.h
  src/context/backgroundcontext.h
  src/context/CurrentRandomDocumentContext.h
  src/context/documentcontext.h
  src/context/fontchangedobservable.h
  src/context/fontcontext.h
  src/context/ifontchangedobserver.h
  )
SET(
  src_context
  src/context/appcontext.cpp
  src/context/backgroundcontext.cpp
  src/context/documentcontext.cpp
  src/context/fontchangedobservable.cpp
  src/context/fontcontext.cpp
  )

SET(
  headers_core
  src/core/configurationmanager.h
  )
SET(
  src_core
  src/core/configurationmanager.cpp
  )

SET(
  headers_iomanager
  src/iomanager/documentfilemanager.h
  src/iomanager/documentloaderdirector.h
  src/iomanager/documentloader.h
  src/iomanager/documentsaverdirector.h
  src/iomanager/documentsaver.h
  src/iomanager/fontfilemanager.h
  src/iomanager/TxtDocumentSaver.hpp
  src/iomanager/xmldocumentloader.h
  src/iomanager/xmldocumentsaver.h
  )
SET(
  src_iomanager
  src/iomanager/documentfilemanager.cpp
  src/iomanager/fontfilemanager.cpp
  src/iomanager/TxtDocumentSaver.cpp
  src/iomanager/xmldocumentloader.cpp
  src/iomanager/xmldocumentsaver.cpp
  )


SET(
  headers_models
  src/models/characterdata.h
  src/models/character.h
  src/models/doc.h
  src/models/font.h
  )
SET(
  src_models
  src/models/characterdata.cpp
  src/models/character.cpp
  src/models/font.cpp
  )

SET(
  headers_models_doc
  src/models/doc/block.h
  src/models/doc/doccharacter.h
  src/models/doc/doccomponentblock.h
  src/models/doc/doccomponent.h
  src/models/doc/docimageblock.h
  src/models/doc/docparagraph.h
  src/models/doc/docstring.h
  src/models/doc/docstyle.h
  src/models/doc/docstylemanager.h
  src/models/doc/doctestblock.h
  src/models/doc/doctextblock.h
  src/models/doc/document.h
  src/models/doc/doczone.h
  src/models/doc/element.h
  src/models/doc/leaf.h
  src/models/doc/node.h
  src/models/doc/nodeofleafs.h
  src/models/doc/nodeofnodes.h
  src/models/doc/page.h
  )
SET(
  src_models_doc
  src/models/doc/block.cpp
  src/models/doc/doccomponentblock.cpp
  src/models/doc/docimageblock.cpp
  src/models/doc/docparagraph.cpp
  src/models/doc/docstring.cpp
  src/models/doc/docstyle.cpp
  src/models/doc/docstylemanager.cpp
  src/models/doc/doctestblock.cpp
  src/models/doc/doctextblock.cpp
  src/models/doc/document.cpp
  src/models/doc/doczone.cpp
  src/models/doc/element.cpp
  src/models/doc/leaf.cpp
  src/models/doc/page.cpp
)  

SET(
  headers_mvc
  src/mvc/icontroller.h
  src/mvc/iview.h
  )

SET(
  headers_patterns
  src/patterns/observable.h
  src/patterns/observer.h
  src/patterns/singleton.h
  )
SET(
  src_patterns
  src/patterns/observable.cpp
  )

SET(
  headers_utils
  src/Utils/connectedcomponentextraction.h
  src/Utils/convertor.h
  src/Utils/ImageUtils.hpp
  )
SET(
  src_utils
  src/Utils/connectedcomponentextraction.cpp
  src/Utils/convertor.cpp
  src/Utils/ImageUtils.cpp
  )

SET(
  headers_global
  src/context.h
  src/core.h
  src/framework_global.h
  src/framework.h
  src/iomanager.h
  src/models.h
  src/mvc.h
  src/patterns.h
  )

SET(
  DocCreatorFramework_sources
  ${src_context} ${src_core} ${src_iomanager} ${src_models} ${src_models_doc} ${src_patterns} ${src_utils}
  )



ADD_LIBRARY(DocCreatorFramework STATIC ${DocCreatorFramework_sources} ${MOC})
TARGET_INCLUDE_DIRECTORIES(DocCreatorFramework PUBLIC src)
TARGET_COMPILE_OPTIONS(DocCreatorFramework PRIVATE ${WARNING_FLAGS})
TARGET_LINK_LIBRARIES(DocCreatorFramework ${QT_LIBRARIES} ${ThirdParty_LIBS} DocCreatorDegradations)
SET_TARGET_PROPERTIES(DocCreatorFramework PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )




INSTALL(FILES ${headers_global} DESTINATION include/DocCreatorFramework)
INSTALL(FILES ${headers_context} DESTINATION include/DocCreatorFramework/context)
INSTALL(FILES ${headers_core} DESTINATION include/DocCreatorFramework/core)
INSTALL(FILES ${headers_iomanager} DESTINATION include/DocCreatorFramework/iomanager)
INSTALL(FILES ${headers_models} DESTINATION include/DocCreatorFramework/models)
INSTALL(FILES ${headers_models_doc} DESTINATION include/DocCreatorFramework/models/doc)
INSTALL(FILES ${headers_mvc} DESTINATION include/DocCreatorFramework/mvc)
INSTALL(FILES ${headers_patterns} DESTINATION include/DocCreatorFramework/patterns)
INSTALL(FILES ${Utils_headers} DESTINATION include/DocCreatorFramework/Utils)

INSTALL(TARGETS DocCreatorFramework
        #FRAMEWORK DESTINATION /Library/Frameworks
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

