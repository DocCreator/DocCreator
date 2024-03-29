#include <cassert>
#include <iostream>
#include <string>
#include <vector>



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

    while ((entry = readdir(dp)) != nullptr) {

      if (strncmp(entry->d_name, ".", 1) != 0
	  && strncmp(entry->d_name, "..", 2) != 0) {
	entries.push_back(entry->d_name);
      }
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
      if (strncmp(entry, ".", 1) != 0
	  && strncmp(entry, "..", 2) != 0) {
		//std::cerr<<entry<<"\n";
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




