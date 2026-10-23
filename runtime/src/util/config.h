#pragma once
#include <string>

namespace rt {

struct Paths {
    std::string runtimeDir; // directorio runtime\ (contiene config\paths.ini)
    std::string isoRoot;
    std::string image;
    std::string xex;
    std::string cacheDir;
    std::string hddDir;
    std::string userdataDir;
};

// Localiza runtime\config\paths.ini subiendo desde el directorio del ejecutable y lo carga.
bool LoadPaths(Paths& out, std::string* err);
const Paths& GetPaths();
void SetUserdataDir(const std::string& dir);

} // namespace rt
