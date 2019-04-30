#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <framework_global.h>

#include <vector>
#include <string>

namespace dc {

  /*
    list files in @a dirname (except "." and "..", and not recursively)
    
    returned filenames are not with absolute path.
  */
  extern FRAMEWORK_EXPORT std::vector<std::string>
  listDirectory(const std::string &dirname);
  
  /*
    Return path.
    On linux/mac, it will be @a dirname + "/" + @a filename.
  */
  extern FRAMEWORK_EXPORT std::string
  makePath(const std::string &dirname, const std::string &filename);

} //namespace dc

#endif /* ! FILE_UTILS_HPP */
