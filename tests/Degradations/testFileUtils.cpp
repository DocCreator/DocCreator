#include <fstream>

#include <catch2/catch_test_macros.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "Degradations/FileUtils.hpp"

#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif

/*
  The stain and phantom pattern directories are user-provided, and every
  entry found in them used to be handed to cv::imread().
*/

namespace {

  const char *DIRNAME = "tmp_testFileUtils_dir";
  //a directory of its own, so that the files written here are not listed by
  //the other tests
  const char *DOTDIRNAME = "tmp_testFileUtils_dotfiles";

  void
  writeFile(const std::string &path, const std::string &content)
  {
    std::ofstream out(path.c_str(), std::ios::binary);
    REQUIRE( out.good() );
    out << content;
  }

  bool
  contains(const std::vector<std::string> &entries, const std::string &name)
  {
    return std::find(entries.begin(), entries.end(), name) != entries.end();
  }

} //namespace

TEST_CASE("hasImageExtension()")
{
  REQUIRE( dc::hasImageExtension("stain.png") );
  REQUIRE( dc::hasImageExtension("stain.PNG") );
  REQUIRE( dc::hasImageExtension("stain.JpEg") );
  REQUIRE( dc::hasImageExtension("a.b.c.tiff") );
  REQUIRE( dc::hasImageExtension(".hidden.png") );

  REQUIRE_FALSE( dc::hasImageExtension("notes.txt") );
  REQUIRE_FALSE( dc::hasImageExtension("archive.tar.gz") );
  REQUIRE_FALSE( dc::hasImageExtension("Makefile") );
  REQUIRE_FALSE( dc::hasImageExtension("") );
  REQUIRE_FALSE( dc::hasImageExtension(".") );
  REQUIRE_FALSE( dc::hasImageExtension("..") );
  REQUIRE_FALSE( dc::hasImageExtension("png") );
  REQUIRE_FALSE( dc::hasImageExtension("image.png.exe") );
}

TEST_CASE("listImageFiles() only returns the image files")
{
#ifdef WIN32
  _mkdir(DIRNAME);
#else
  mkdir(DIRNAME, 0755);
#endif
  const std::string sub = dc::makePath(DIRNAME, "subdir.png");
#ifdef WIN32
  _mkdir(sub.c_str());
#else
  mkdir(sub.c_str(), 0755);
#endif

  const cv::Mat img(8, 8, CV_8UC3, cv::Scalar::all(128));
  REQUIRE( cv::imwrite(dc::makePath(DIRNAME, "stain.png"), img) );
  REQUIRE( cv::imwrite(dc::makePath(DIRNAME, "stain2.PNG"), img) );
  writeFile(dc::makePath(DIRNAME, "notes.txt"), "not an image");
  writeFile(dc::makePath(DIRNAME, "noextension"), "not an image either");

  const std::vector<std::string> all = dc::listDirectory(DIRNAME);
  const std::vector<std::string> images = dc::listImageFiles(DIRNAME);

  //listDirectory() returns everything, the subdirectory included
  REQUIRE( contains(all, "notes.txt") );
  REQUIRE( contains(all, "subdir.png") );

  REQUIRE( contains(images, "stain.png") );
  REQUIRE( contains(images, "stain2.PNG") );
  REQUIRE_FALSE( contains(images, "notes.txt") );
  REQUIRE_FALSE( contains(images, "noextension") );
  //a directory named like an image is not an image
  REQUIRE_FALSE( contains(images, "subdir.png") );
  REQUIRE( images.size() == 2 );

#ifndef WIN32
  //a symbolic link to an image is one
  const std::string link = dc::makePath(DIRNAME, "link.png");
  unlink(link.c_str());
  REQUIRE( symlink("stain.png", link.c_str()) == 0 );
  REQUIRE( dc::listImageFiles(DIRNAME).size() == 3 );

  //a dangling one is not
  unlink(link.c_str());
  REQUIRE( symlink("nowhere.png", link.c_str()) == 0 );
  REQUIRE( dc::listImageFiles(DIRNAME).size() == 2 );
#endif
}

TEST_CASE("listDirectory() only skips \".\" and \"..\"")
{
#ifdef WIN32
  _mkdir(DOTDIRNAME);
#else
  mkdir(DOTDIRNAME, 0755);
#endif
  writeFile(dc::makePath(DOTDIRNAME, ".hidden"), "a dotfile");
  writeFile(dc::makePath(DOTDIRNAME, ".hidden.png"), "a dotfile too");

  const std::vector<std::string> all = dc::listDirectory(DOTDIRNAME);

  //strncmp(d_name, ".", 1) skipped every name starting with a dot
  REQUIRE( contains(all, ".hidden") );
  REQUIRE( contains(all, ".hidden.png") );

  REQUIRE_FALSE( contains(all, ".") );
  REQUIRE_FALSE( contains(all, "..") );
}

TEST_CASE("listImageFiles() on a directory that does not exist")
{
  REQUIRE( dc::listImageFiles("/nonexistent_directory_for_the_tests").empty() );
}


TEST_CASE("makePath()")
{
  //joined with the separator of the system, which on Windows is "\\"
#ifdef _WIN32
  const std::string sep = "\\";
#else
  const std::string sep = "/";
#endif
  REQUIRE( dc::makePath("dir", "file.png") == "dir" + sep + "file.png" );
  REQUIRE( dc::makePath("a" + sep + "b", "c") == "a" + sep + "b" + sep + "c" );
  //no second separator after a directory that ends with one
  REQUIRE( dc::makePath("dir" + sep, "file.png") == "dir" + sep + "file.png" );
  //an empty directory is the current one: the file name alone, not a path
  // from the root
  REQUIRE( dc::makePath("", "file.png") == "file.png" );
}

TEST_CASE("listing a directory that does not exist gives nothing")
{
  REQUIRE( dc::listDirectory("no_such_directory_of_testFileUtils").empty() );
  REQUIRE( dc::listImageFiles("no_such_directory_of_testFileUtils").empty() );
}
