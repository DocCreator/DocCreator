#include <algorithm> //std::transform
#include <cctype> //std::tolower
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

#include "FileUtils.hpp"

/*
  B: this file had two implementations of listDirectory() and
  listImageFiles(), one with opendir()/readdir()/stat() and one with
  FindFirstFile()/FindNextFile(), and two of makePath(). The Windows one
  was never built by the Linux builds the tests run in, limited paths to
  MAX_PATH, and printed its errors with _tprintf() rather than like the
  other. std::filesystem does both, in one implementation.

  Errors are reported through std::error_code rather than exceptions: a
  directory that cannot be read gives an empty list and a message, as
  before.

  B: the paths are UTF-8 (fs::u8path(), path::u8string()), as the callers
  give them (Qt's toStdString(), the python binding). With std::string as
  is, MSVC used the ANSI code page: path::string() threw std::system_error
  for a file whose name it cannot represent (a Chinese name with the
  Western code page 1252, for example), out of the degradation that listed
  the directory, and a UTF-8 directory name with an accent was not found.
  (cv::imread() still uses the ANSI code page on Windows: such a file is
  listed, and reported unreadable by the caller.)
  In C++20, u8path() is deprecated and u8string() gives a std::u8string.
*/

namespace fs = std::filesystem;

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

  namespace {

    /*
      Call @a f with each entry of @a dirname, not recursively; "." and
      ".." are not entries. @return false, after a message on the standard
      error, if the directory cannot be read.
    */
    template <typename F>
    bool
    forEachEntry(const std::string &dirname, F f)
    {
      std::error_code ec;
      fs::directory_iterator it(fs::u8path(dirname), ec);
      const fs::directory_iterator end;
      for ( ; ! ec && it != end; it.increment(ec)) {
	f(*it);
      }
      if (ec) {
	std::cerr<<"ERROR: unable to read directory: "<<dirname<<": "<<ec.message()<<"\n";
	return false;
      }
      return true;
    }

  } //anonymous namespace

  std::vector<std::string>
  listDirectory(const std::string &dirname)
  {
    std::vector<std::string> entries;
    if (! forEachEntry(dirname, [&](const fs::directory_entry &entry) {
	  entries.push_back(entry.path().filename().u8string());
	})) {
      entries.clear();
    }
    return entries;
  }

  std::vector<std::string>
  listImageFiles(const std::string &dirname)
  {
    std::vector<std::string> entries;
    if (! forEachEntry(dirname, [&](const fs::directory_entry &entry) {
	  const std::string name = entry.path().filename().u8string();
	  if (! hasImageExtension(name)) {
	    return;
	  }
	  //a regular file, or a link to one: links are followed, and a link
	  // to nothing is not a regular file
	  std::error_code ec;
	  if (entry.is_regular_file(ec) && ! ec) {
	    entries.push_back(name);
	  }
	})) {
      entries.clear();
    }
    return entries;
  }

  std::string
  makePath(const std::string &dirname,
	   const std::string &filename)
  {
    return (fs::u8path(dirname) / fs::u8path(filename)).u8string();
  }

} //namespace dc
