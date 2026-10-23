#include "config.h"

#include <windows.h>

#include <fstream>
#include <map>

namespace rt {

static Paths g_paths;

static std::string Trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n"), b = s.find_last_not_of(" \t\r\n");
    return a == std::string::npos ? "" : s.substr(a, b - a + 1);
}

static bool Exists(const std::string& p) { return GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }

static std::string Canon(const std::string& p) {
    char buf[MAX_PATH * 4];
    DWORD n = GetFullPathNameA(p.c_str(), sizeof(buf), buf, nullptr);
    return n ? std::string(buf, n) : p;
}

bool LoadPaths(Paths& out, std::string* err) {
    char exe[MAX_PATH * 4];
    DWORD n = GetModuleFileNameA(nullptr, exe, sizeof(exe));
    std::string dir(exe, n);
    std::string found;
    for (int i = 0; i < 8; ++i) {
        size_t p = dir.find_last_of("\\/");
        if (p == std::string::npos) break;
        dir = dir.substr(0, p);
        if (Exists(dir + "\\config\\paths.ini")) { found = dir; break; }
    }
    if (found.empty()) { if (err) *err = "config\\paths.ini no encontrado"; return false; }
    std::ifstream f(found + "\\config\\paths.ini");
    std::map<std::string, std::string> kv;
    std::string line;
    while (std::getline(f, line)) {
        line = Trim(line);
        if (line.empty() || line[0] == ';' || line[0] == '[') continue;
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        kv[Trim(line.substr(0, eq))] = Trim(line.substr(eq + 1));
    }
    auto get = [&](const char* k) { return Canon(found + "\\" + kv[k]); };
    out.runtimeDir = found;
    out.isoRoot = get("iso_root");
    if (kv.count("iso_root_es_it") && !kv["iso_root_es_it"].empty()) {
        std::string r = Canon(found + "\\" + kv["iso_root_es_it"]);
        if (Exists(r + "\\default.xex")) out.isoRootEsIt = r;
    }
    out.image = get("image");
    out.xex = get("xex");
    out.cacheDir = get("cache");
    out.hddDir = get("hdd");
    out.userdataDir = Canon(found + "\\userdata");
    g_paths = out;
    return true;
}

const Paths& GetPaths() { return g_paths; }

void SetUserdataDir(const std::string& dir) {
    if (dir.empty()) return;
    g_paths.userdataDir = Canon(dir);
    g_paths.cacheDir = Canon(g_paths.userdataDir + "\\cache");
    g_paths.hddDir = Canon(g_paths.userdataDir + "\\hdd");
}

} // namespace rt
