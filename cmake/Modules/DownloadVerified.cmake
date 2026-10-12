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
