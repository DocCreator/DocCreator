#ifndef OBJ_HPP
#define OBJ_HPP

#include "Mesh.hpp"
#include <string>

/*
 As OBJ files do not have a magic number,
 they must partially parsed to kno if they are OBJ files.
 Tus it is rather slow.

 */
bool
isOBJFile(const std::string &filename);

bool
readOBJ(const std::string &filename, Mesh &mesh);

bool
writeOBJ(const std::string &filename, const Mesh &mesh);

#endif /* ! OBJ_HPP */
