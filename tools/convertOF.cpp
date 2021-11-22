#include <cassert>
#include <cstdlib>
#include <iostream>

#include "models/font.h"
#include "iomanager/fontfilemanager.h"


static const std::string OF_EXT = "of";
static const std::string BOF_EXT = "bof";

static std::string
getFilenameWithoutExtension(const std::string &filename)
{
  std::string::size_type pos = filename.rfind('.');
  if (pos != std::string::npos)
    return std::string(filename, 0, pos + 1);
  return std::string();
}

static std::string
getExtension(const std::string &filename)
{
  std::string::size_type pos = filename.rfind('.');
  if (pos != std::string::npos)
    return std::string(filename, pos + 1);
  return std::string();
}

static std::string
tolower(const std::string &s)
{
  //std::transform(s.begin(), s.end(), s.begin(), (int(*)(int))std::tolower);

  std::string so = s;
  for (auto &c : so)
    c = tolower(c);
  return so;
}

int
main(int argc, char **argv)
{
  if (argc != 2 && argc != 3) {
    std::cerr << "Usage: " << argv[0]
              << " inputFontFilename [outputFontFilename]\n";
    std::cerr << "If outputFontFilename is not provided, output filename will "
                 "be input filename with a different extension\n";
    exit(10);
  }

  assert(argc >= 2);
  std::string inputFilename = argv[1];
  std::string inputFormat = tolower(getExtension(inputFilename));

  std::string outputFilename;
  std::string outputFormat;
  if (argc == 3) {
    outputFilename = argv[2];
    outputFormat = tolower(getExtension(outputFilename));

    if (outputFormat == inputFormat) {
      std::cerr << "ERROR: same format for input & output. Nothing done.\n";
      exit(11);
    }
    if (inputFilename == outputFilename) {
      std::cerr << "WARNING: same file for input and output. Nothing done.\n";
      return 0;
    }
    
  }
  else {
    outputFilename = getFilenameWithoutExtension(inputFilename);
    if (inputFormat == OF_EXT) {
      outputFilename += BOF_EXT;
      outputFormat = BOF_EXT;
    }
    else if (inputFormat == BOF_EXT) {
      outputFilename += OF_EXT;
      outputFormat = OF_EXT;
    }
    else {
      std::cerr << "ERROR: unknown extension for input filename: "
                << inputFilename << "\n";
      exit(12);
    }
  }

  if (inputFormat == OF_EXT) {
    Models::Font *font = IOManager::FontFileManager::fontFromXml(QString::fromStdString(inputFilename));
    if (font == nullptr) {
      std::cerr<<"ERROR: unable to read font: "<<inputFilename<<"\n";
      exit(13);
    }
    const bool writeOk = IOManager::FontFileManager::fontToBinary(font, QString::fromStdString(outputFilename));
    if (! writeOk) {
      std::cerr<<"ERROR: unable to write font: "<<outputFilename<<"\n";
      exit(14);
    }
  }
  else if (inputFormat == BOF_EXT) {
    Models::Font *font = IOManager::FontFileManager::fontFromBinary(QString::fromStdString(inputFilename));
    if (font == nullptr) {
      std::cerr<<"ERROR: unable to read font: "<<inputFilename<<"\n";
      exit(13);
    }
    const bool writeOk = IOManager::FontFileManager::fontToXml(font, QString::fromStdString(outputFilename));
    if (! writeOk) {
      std::cerr<<"ERROR: unable to write font: "<<outputFilename<<"\n";
      exit(14);
    }
  }

  if (argc < 3) {
    std::cout << "wrote " << outputFilename << "\n";
  }

  return 0;
}
