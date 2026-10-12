#include <algorithm> //std::transform
#include <cassert>
#include <cctype> //std::tolower
#include <iostream>
#include <string>
#include <vector>

#include "FileUtils.hpp"




namespace dc {

  /*
    Extensions of the image formats cv::imread() handles
    (cf opencv2/imgcodecs.hpp).
  */
  static const char *IMAGE_EXTENSIONS[] = {
    ".bmp", ".dib",  ".exr",  ".hdr", ".jp2", ".jpe", ".jpeg", ".jpg",
    ".pbm", ".pgm",  ".pic",  ".png", ".pnm", ".ppm", ".pxm",  ".ras",
    ".sr",  ".tif",  ".tiff", ".webp"
  };

  bool
  hasImageExtension(const std::string &filename)
  {
    const size_t pos = filename.rfind('.');
    if (pos == std::string::npos) {
      return false;
    }

    std::string ext = filename.substr(pos);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
      return static_cast<char>(std::tolower(c));
    });

    for (const char *imageExt : IMAGE_EXTENSIONS) {
      if (ext == imageExt) {
	return true;
      }
    }
    return false;
  }

} //namespace dc


#ifndef WIN32

  //C/POSIX versions

#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace dc {

  std::vector<std::string>
  listDirectory(const std::string &dirname)
  {
    std::vector<std::string> entries;
  
    struct dirent *entry = nullptr;
    DIR *dp = nullptr;

    dp = opendir(dirname.c_str());
    if (dp == nullptr) {
      std::cerr<<"ERROR: unable to open directory: "<<dirname<<"\n";
      return entries;
    }

    while ((entry = readdir(dp)) != nullptr) {

      //B: strncmp(d_name, ".", 1) skipped every file whose name begins with
      // a dot, and made the comparison with ".." unreachable.
      const std::string name = entry->d_name;
      if (name != "." && name != "..") {
	entries.push_back(entry->d_name);
      }
    }

    closedir(dp);

    return entries;
  }

  std::vector<std::string>
  listImageFiles(const std::string &dirname)
  {
    std::vector<std::string> entries;

    DIR *dp = opendir(dirname.c_str());
    if (dp == nullptr) {
      std::cerr<<"ERROR: unable to open directory: "<<dirname<<"\n";
      return entries;
    }

    struct dirent *entry = nullptr;
    while ((entry = readdir(dp)) != nullptr) {

      if (! hasImageExtension(entry->d_name)) {
	continue;
      }

      //d_type is not provided by every filesystem
      if (entry->d_type != DT_UNKNOWN && entry->d_type != DT_REG
	  && entry->d_type != DT_LNK) {
	continue;
      }

      struct stat st;
      const std::string path = makePath(dirname, entry->d_name);
      if (stat(path.c_str(), &st) != 0 || ! S_ISREG(st.st_mode)) {
	continue;
      }

      entries.push_back(entry->d_name);
    }

    closedir(dp);

    return entries;
  }

  std::string
  makePath(const std::string &dirname,
	   const std::string &filename)
  {
    return (dirname + "/" ) + filename;
  }

} //namespace dc


#else

  // Win32 versions

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <strsafe.h> //StringCchLength
#include <tchar.h> //_tprintf

namespace dc {

  std::vector<std::string>
  listDirectory(const std::string &dirname)
  {
    std::vector<std::string> entries;

	std::string dirname2 = dirname + "\\*";
   // _tprintf(TEXT("\nDEBUG dirname2=%s\n"), dirname2.c_str());

    size_t length_of_arg;
    const HRESULT res = StringCchLength(dirname2.c_str(), MAX_PATH, &length_of_arg);
    if (FAILED(res) || length_of_arg > (MAX_PATH - 1)) {
      _tprintf(TEXT("\nDirectory path is too long.\n"));
      return entries;
    }
  
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile(dirname2.c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
      _tprintf(TEXT("FindFirstFile failed (%lu)\n"), GetLastError());
      return entries;
    }

    do {
      /*
	if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	_tprintf(TEXT("  %s   <DIR>\n"), FindFileData.cFileName);
	else
	_tprintf(TEXT("  %s \n"), FindFileData.cFileName);
      */
      const char *entry = FindFileData.cFileName;
      //B: cf the POSIX version above.
      const std::string name = entry;
      if (name != "." && name != "..") {
		//std::cerr<<entry<<"\n";
		entries.push_back(entry);
      }
    
    }
    while (FindNextFile(hFind, &FindFileData) != 0);

    FindClose(hFind);

    return entries;
  }

  std::vector<std::string>
  listImageFiles(const std::string &dirname)
  {
    std::vector<std::string> entries;

    std::string dirname2 = dirname + "\\*";

    size_t length_of_arg;
    const HRESULT res = StringCchLength(dirname2.c_str(), MAX_PATH, &length_of_arg);
    if (FAILED(res) || length_of_arg > (MAX_PATH - 1)) {
      _tprintf(TEXT("\nDirectory path is too long.\n"));
      return entries;
    }

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile(dirname2.c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
      _tprintf(TEXT("FindFirstFile failed (%lu)\n"), GetLastError());
      return entries;
    }

    do {
      if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
	continue;
      }
      const char *entry = FindFileData.cFileName;
      if (hasImageExtension(entry)) {
	entries.push_back(entry);
      }
    }
    while (FindNextFile(hFind, &FindFileData) != 0);

    FindClose(hFind);

    return entries;
  }

  std::string
  makePath(const std::string &dirname,
	   const std::string &filename)
  {
    return dirname + "\\" + filename;
  }

} //namespace dc

#endif //Win32




