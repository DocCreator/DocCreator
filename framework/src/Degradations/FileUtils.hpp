#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <framework_global.h>

#include <vector>
#include <string>

namespace dc {

  /*
    list the entries of @a dirname (except "." and "..", and not
    recursively). Sub-directories are listed as well.
    
    returned filenames are not with absolute path.
  */
  extern FRAMEWORK_EXPORT std::vector<std::string>
  listDirectory(const std::string &dirname);
  
  /*
    list the image files in @a dirname (not recursively).

    Only the regular files whose extension is one of the image formats
    handled by OpenCV are returned, as the content of the directory is
    given to cv::imread(): there is no reason to hand every file found in
    a user-provided directory to the image decoders.

    returned filenames are not with absolute path.
  */
  extern FRAMEWORK_EXPORT std::vector<std::string>
  listImageFiles(const std::string &dirname);

  /*
    @return whether @a filename has the extension of an image format
    handled by OpenCV (the comparison ignores the case).
  */
  extern FRAMEWORK_EXPORT bool
  hasImageExtension(const std::string &filename);

  /*
    Return path.
    On linux/mac, it will be @a dirname + "/" + @a filename.
  */
  extern FRAMEWORK_EXPORT std::string
  makePath(const std::string &dirname, const std::string &filename);

} //namespace dc

#endif /* ! FILE_UTILS_HPP */
