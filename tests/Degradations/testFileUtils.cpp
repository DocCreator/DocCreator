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

TEST_CASE("listImageFiles() on a directory that does not exist")
{
  REQUIRE( dc::listImageFiles("/nonexistent_directory_for_the_tests").empty() );
}
