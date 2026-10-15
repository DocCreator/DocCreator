#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <framework_global.h>

#include <vector>
#include <string>

namespace dc {

  /**
    List the entries of a directory, not recursively: files and
    subdirectories, but not "." and "..".

    If the directory cannot be read, an error is printed on the standard
    error and the list is empty.

    @param dirname  directory to list.
    @return the names of the entries, relative to @a dirname, in the order
    the system gives them.
  */
  extern FRAMEWORK_EXPORT std::vector<std::string>
  listDirectory(const std::string &dirname);
  
  /**
    List the image files of a directory, not recursively.

    Only the regular files whose extension is one of the image formats
    handled by OpenCV are listed (cf. hasImageExtension()): the list is
    meant for cv::imread(), and there is no reason to give every file of a
    user-provided directory to the image decoders.

    If the directory cannot be read, an error is printed on the standard
    error and the list is empty.

    @param dirname  directory to list.
    @return the names of the image files, relative to @a dirname, in the
    order the system gives them.
  */
  extern FRAMEWORK_EXPORT std::vector<std::string>
  listImageFiles(const std::string &dirname);

  /**
    @param filename  file name or path.
    @return whether @a filename has the extension of an image format handled
    by OpenCV. The comparison ignores the case.
  */
  extern FRAMEWORK_EXPORT bool
  hasImageExtension(const std::string &filename);

  /**
    @param dirname  directory.
    @param filename  name of a file in @a dirname.
    @return the path of @a filename in @a dirname, joined with the
    separator of the system ("/", or "\\" on Windows) unless @a dirname
    already ends with one: std::filesystem::path(@a dirname) / @a filename.
    An empty @a dirname gives @a filename.
  */
  extern FRAMEWORK_EXPORT std::string
  makePath(const std::string &dirname, const std::string &filename);

} //namespace dc

#endif /* ! FILE_UTILS_HPP */
