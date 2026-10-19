# DOWNLOAD_VERIFIED(<url> <destination> <sha256> <description>)
#
# Download <url> to <destination> and check it against <sha256>.
#
# The archives fetched at configure time are extracted and compiled, and the
# OCR language models are read by tesseract. Without a hash, whoever is able
# to intercept the connection, or able to change what the hosting service
# serves, chooses what is built on the machine of every developer.
#
# A file already present is checked as well: it may have been downloaded
# before the hash was pinned, or replaced since. It is downloaded again when
# it does not match.
#
# When a version is changed, the expected hash has to be changed too: run
# "sha256sum <the archive>" (or "cmake -E sha256sum") and put the result
# next to the version.

FUNCTION(DOWNLOAD_VERIFIED url destination sha256 description)

  IF(EXISTS ${destination})
    FILE(SHA256 ${destination} actual_sha256)
    IF(actual_sha256 STREQUAL sha256)
      RETURN()
    ENDIF()
    MESSAGE(WARNING "  ${description}: ${destination} does not have its expected SHA256 (${actual_sha256} instead of ${sha256}): downloading it again")
    FILE(REMOVE ${destination})
  ENDIF()

  MESSAGE(STATUS "  downloading ${description}")
  FILE(DOWNLOAD ${url} ${destination}
    EXPECTED_HASH SHA256=${sha256}
    TLS_VERIFY ON
    STATUS status
    LOG log)

  LIST(GET status 0 status_code)
  LIST(GET status 1 status_string)
  IF(NOT status_code EQUAL 0)
    # Do not leave a partial file behind: the next configure would find it
    # in place. (A hash mismatch makes FILE(DOWNLOAD) itself stop the
    # configure; the file it leaves is detected by the check above on the
    # next run, and downloaded again.)
    FILE(REMOVE ${destination})
    MESSAGE(FATAL_ERROR "error: downloading ${url} failed: status code=${status_code} string=${status_string}, log=${log}")
  ENDIF()

ENDFUNCTION()


# FETCH_ARCHIVE_VERIFIED(<url> <sha256> <description> <dirname> <out_var>)
#
# Download the archive <url> (cf DOWNLOAD_VERIFIED()), whose top directory is
# <dirname>, extract it in the build tree, and set <out_var> to the
# directory extracted.
#
# B: the archives were extracted in the source tree, which fails when it is
# read-only, and the result was not checked: an extraction that failed, or
# was interrupted, left a partial directory, which the next configuration
# took as complete because it existed. The directory is now complete only
# once a stamp, with the hash of its archive, is written next to it.
FUNCTION(FETCH_ARCHIVE_VERIFIED url sha256 description dirname out_var)
  SET(root "${CMAKE_BINARY_DIR}/thirdparty")
  SET(dir "${root}/${dirname}")
  SET(stamp "${root}/${dirname}.extracted")
  SET(${out_var} "${dir}" PARENT_SCOPE)

  IF(EXISTS "${stamp}" AND IS_DIRECTORY "${dir}")
    FILE(READ "${stamp}" stamped_sha256)
    IF(stamped_sha256 STREQUAL sha256)
      RETURN()
    ENDIF()
  ENDIF()

  FILE(REMOVE "${stamp}")
  FILE(REMOVE_RECURSE "${dir}" "${dir}.tmp")
  GET_FILENAME_COMPONENT(archive_name "${url}" NAME)
  SET(archive "${root}/${dirname}-${archive_name}")
  DOWNLOAD_VERIFIED("${url}" "${archive}" "${sha256}" "${description}")

  FILE(MAKE_DIRECTORY "${dir}.tmp")
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar xzf "${archive}"
    WORKING_DIRECTORY "${dir}.tmp"
    RESULT_VARIABLE result)
  IF(NOT result EQUAL 0 OR NOT IS_DIRECTORY "${dir}.tmp/${dirname}")
    FILE(REMOVE_RECURSE "${dir}.tmp")
    MESSAGE(FATAL_ERROR "error: extracting ${archive} failed (${result}), or it has no ${dirname}/ directory")
  ENDIF()
  FILE(RENAME "${dir}.tmp/${dirname}" "${dir}")
  FILE(REMOVE_RECURSE "${dir}.tmp")
  FILE(WRITE "${stamp}" "${sha256}")
  MESSAGE(STATUS "  ${description} extracted in ${dir}")
ENDFUNCTION()
