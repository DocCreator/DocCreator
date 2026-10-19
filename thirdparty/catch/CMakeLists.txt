INCLUDE(DownloadVerified)

SET(CATCH2_VERSION "3.14.0")
#SHA256 of v${CATCH2_VERSION}.tar.gz: change it when the version changes
SET(CATCH2_SHA256 "ba2a939efead3c833c499cf487e185762f419a71d30158cd1b43c6079c586490")
FETCH_ARCHIVE_VERIFIED("https://github.com/catchorg/Catch2/archive/refs/tags/v${CATCH2_VERSION}.tar.gz"
                       ${CATCH2_SHA256}
                       "Catch2 v${CATCH2_VERSION}"
                       "Catch2-${CATCH2_VERSION}"
                       CATCH2_SRC_DIR)

ADD_SUBDIRECTORY(${CATCH2_SRC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/Catch2)
