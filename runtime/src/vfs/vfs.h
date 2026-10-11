#pragma once
// Sistema de ficheros virtual: traduce rutas guest (Xbox 360) a rutas host.
// Montajes (sin distinguir mayusculas; '/' equivale a '\'; se rechaza '..'; se elimina el prefijo "\??\"):
//   game:  app:  d:  \Device\Cdrom0                 -> iso_root (solo lectura)
//   cache: cache0: cache1: \Device\Harddisk0\Cache0/1 \Device\cache0/1 -> userdata\cache
//   hdd:   \Device\Harddisk0\partition0              -> userdata\hdd
// Los alias creados con ObCreateSymbolicLink se resuelven antes de aplicar los montajes.
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace rt {

struct VfsMount {
    std::string prefix;   // minusculas, p. ej. "game:" o "\device\cdrom0"
    std::string hostRoot; // ruta host sin barra final
    bool readOnly;
};

struct VfsResolved {
    uint32_t status = 0;     // NTSTATUS de la resolucion (0 = ruta valida)
    std::string host;        // ruta host (con las mayusculas reales si existe)
    std::string normalized;  // ruta guest normalizada (minusculas)
    bool readOnly = false;
    bool exists = false;
    bool isDir = false;
    bool mountRoot = false;  // la ruta es la raiz del montaje
};

class Vfs {
public:
    static Vfs& Get();
    void Init(const std::string& isoRoot, const std::string& cacheDir, const std::string& hddDir);
    VfsResolved Resolve(const std::string& guestPath);
    void AddAlias(const std::string& name, const std::string& target);
    bool RemoveAlias(const std::string& name);
    static std::string Normalize(const std::string& p); // minusculas, '/'->'\', sin "\??\" inicial

private:
    std::mutex mu_;
    std::vector<VfsMount> mounts_;
    std::map<std::string, std::string> aliases_;
};

} // namespace rt
