#include <cassert>
#include <vector>
#include <string>
#include <iostream>


#ifndef WIN32

  //C/POSIX versions

#include <cstdio>
#include <cstring>
#include <dirent.h>

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
    assert(dp != nullptr);

    while ((entry = readdir(dp))) {

      if (strncmp(entry->d_name, ".", 1) != 0
	  && strncmp(entry->d_name, "..", 2) != 0) {
	entries.push_back(entry->d_name);
      }
    }

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

namespace dc {

  std::vector<std::string>
  listDirectory(const std::string &dirname)
  {
    std::vector<std::string> entries;

    size_t length_of_arg;
    StringCchLength(dirname.c_path(), MAX_PATH, &length_of_arg);
    if (length_of_arg > (MAX_PATH - 1)) {
      _tprintf(TEXT("\nDirectory path is too long.\n"));
      return entries;
    }
  
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile(dirname.c_path(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
      printf ("FindFirstFile failed (%d)\n", GetLastError());
      return entries;
    }

    do {
      /*
	if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	_tprintf(TEXT("  %s   <DIR>\n"), FindFileData.cFileName);
	else
	_tprintf(TEXT("  %s \n"), FindFileData.cFileName);
      */
      if (strncmp(entry->d_name, ".", 1) != 0
	  && strncmp(entry->d_name, "..", 2) != 0) {
	std::cerr<<entry->d_name<<"\n";
	entries.push_back(entry->d_name);
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




