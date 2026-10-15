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
      fs::directory_iterator it(dirname, ec);
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
	  entries.push_back(entry.path().filename().string());
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
	  const std::string name = entry.path().filename().string();
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
    return (fs::path(dirname) / filename).string();
  }

} //namespace dc
