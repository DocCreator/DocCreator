#include <cstdlib>
#include <iostream>

#include "Mesh.hpp"
#include "brs.hpp"
#include "obj.hpp"

static const std::string OBJ_EXT = "OBJ";
static const std::string BRS_EXT = "BRS";

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
toupper(const std::string &s)
{
  //std::transform(s.begin(), s.end(), s.begin(), (int(*)(int))std::toupper);

  std::string so = s;
  for (auto &c : so)
    c = toupper(c);
  return so;
}

int
main(int argc, char **argv)
{
  if (argc != 2 && argc != 3) {
    std::cerr << "Usage: " << argv[0]
              << " inputMeshFilename [outputMeshFilename]\n";
    std::cerr << "If outputMeshFilename is not provided, output filename will "
                 "be input filename with a different extension\n";
    exit(10);
  }

  assert(argc >= 2);
  std::string inputFilename = argv[1];
  std::string inputFormat = toupper(getExtension(inputFilename));

  std::string outputFilename;
  std::string outputFormat;
  if (argc == 3) {
    outputFilename = argv[2];
    outputFormat = toupper(getExtension(outputFilename));

    if (outputFormat == inputFormat) {
      std::cerr << "ERROR: same format for input & output. Nothing done.\n";
      exit(11);
    }

  } else {
    outputFilename = getFilenameWithoutExtension(inputFilename);
    if (inputFormat == OBJ_EXT) {
      outputFilename += "brs";
      outputFormat = BRS_EXT;
    } else if (inputFormat == BRS_EXT) {
      outputFilename += "obj";
      outputFormat = OBJ_EXT;
    } else {
      std::cerr << "ERROR: unknown extension for input filename: "
                << inputFilename << "\n";
      exit(12);
    }
  }

  Mesh m;
  bool loadOk = false;
  if (inputFormat == OBJ_EXT) {
    loadOk = readOBJ(inputFilename, m);
  } else if (inputFormat == BRS_EXT) {
    loadOk = readBRS(inputFilename, m);
  }

  if (!loadOk) {
    std::cerr << "ERROR: unable to read input filename: " << inputFilename
              << "\n";
    exit(13);
  }

  bool saveOk = false;
  if (outputFormat == OBJ_EXT) {
    saveOk = writeOBJ(outputFilename, m);
  } else if (outputFormat == BRS_EXT) {
    saveOk = writeBRS(outputFilename, m);
  }

  if (!saveOk) {
    std::cerr << "ERROR: unable to write output filename: " << outputFilename
              << "\n";
    exit(13);
  }

  if (argc < 3) {
    std::cout << "wrote " << outputFilename << "\n";
  }

  return 0;
}
