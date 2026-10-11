#include "vfs.h"

#include <windows.h>

#include <algorithm>
#include <cctype>

namespace rt {

constexpr uint32_t ST_OBJECT_NAME_INVALID = 0xC0000033;
constexpr uint32_t ST_OBJECT_NAME_NOT_FOUND_ = 0xC0000034;
constexpr uint32_t ST_OBJECT_PATH_NOT_FOUND_ = 0xC000003A;

Vfs& Vfs::Get() {
    static Vfs v;
    return v;
}

static void EnsureDir(const std::string& p) {
    // crea la ruta completa (userdata puede no existir)
    std::string cur;
    for (size_t i = 0; i <= p.size(); ++i) {
        if (i == p.size() || p[i] == '\\') {
            if (cur.size() > 2) CreateDirectoryA(cur.c_str(), nullptr);
        }
        if (i < p.size()) cur.push_back(p[i]);
    }
}

void Vfs::Init(const std::string& isoRoot, const std::string& cacheDir, const std::string& hddDir) {
    std::lock_guard<std::mutex> g(mu_);
    EnsureDir(cacheDir);
    EnsureDir(hddDir);
    mounts_.clear();
    for (const char* p : {"game:", "app:", "d:", "\\device\\cdrom0"}) mounts_.push_back({p, isoRoot, true});
    for (const char* p : {"cache:", "cache0:", "cache1:", "\\device\\harddisk0\\cache0", "\\device\\harddisk0\\cache1",
                          "\\device\\cache0", "\\device\\cache1"})
        mounts_.push_back({p, cacheDir, false});
    for (const char* p : {"hdd:", "\\device\\harddisk0\\partition0"}) mounts_.push_back({p, hddDir, false});
}

std::string Vfs::Normalize(const std::string& in) {
    std::string s;
    s.reserve(in.size());
    for (char c : in) {
        c = (c == '/') ? '\\' : (char)std::tolower((unsigned char)c);
        if (c == '\\' && !s.empty() && s.back() == '\\' && s.size() > 2) continue; // colapsa "\\" (tras el prefijo)
        s.push_back(c);
    }
    while (s.compare(0, 4, "\\??\\") == 0) s.erase(0, 4);
    return s;
}

void Vfs::AddAlias(const std::string& name, const std::string& target) {
    std::lock_guard<std::mutex> g(mu_);
    aliases_[Normalize(name)] = Normalize(target);
}

bool Vfs::RemoveAlias(const std::string& name) {
    std::lock_guard<std::mutex> g(mu_);
    return aliases_.erase(Normalize(name)) != 0;
}

static bool HasPrefix(const std::string& s, const std::string& pre) {
    if (s.size() < pre.size() || s.compare(0, pre.size(), pre) != 0) return false;
    if (s.size() == pre.size()) return true;
    return s[pre.size()] == '\\' || pre.back() == ':'; // "game:foo" y "game:\foo"
}

VfsResolved Vfs::Resolve(const std::string& guestPath) {
    VfsResolved r;
    std::string s = Normalize(guestPath);
    {
        std::lock_guard<std::mutex> g(mu_);
        for (int depth = 0; depth < 8; ++depth) {
            bool changed = false;
            for (auto& kv : aliases_) {
                if (HasPrefix(s, kv.first)) {
                    s = Normalize(kv.second + s.substr(kv.first.size()));
                    changed = true;
                    break;
                }
            }
            if (!changed) break;
        }
        r.normalized = s;
        // ".." prohibido
        size_t pos = 0;
        while (pos <= s.size()) {
            size_t e = s.find('\\', pos);
            if (e == std::string::npos) e = s.size();
            if (s.compare(pos, e - pos, "..") == 0) { r.status = ST_OBJECT_NAME_INVALID; return r; }
            pos = e + 1;
        }
        const VfsMount* best = nullptr;
        for (auto& m : mounts_)
            if (HasPrefix(s, m.prefix) && (!best || m.prefix.size() > best->prefix.size())) best = &m;
        if (!best) { r.status = ST_OBJECT_PATH_NOT_FOUND_; return r; }
        r.readOnly = best->readOnly;
        // resto de la ruta conservando las mayusculas originales (misma longitud ASCII)
        std::string orig = guestPath;
        for (auto& c : orig) if (c == '/') c = '\\';
        // elimina "\??\" y el prefijo del montaje en la ruta original (alineada con s si no hubo alias)
        std::string rest = s.substr(best->prefix.size());
        std::string origRest = rest;
        std::string origNorm = Normalize(orig);
        if (origNorm == s && orig.size() >= s.size()) {
            size_t off = orig.size() - rest.size();
            origRest = orig.substr(off);
        }
        std::string cur = best->hostRoot;
        r.mountRoot = true;
        r.exists = true;
        r.isDir = true;
        size_t p = 0;
        bool missing = false;
        while (p < origRest.size()) {
            size_t e = origRest.find('\\', p);
            if (e == std::string::npos) e = origRest.size();
            std::string comp = origRest.substr(p, e - p);
            p = e + 1;
            if (comp.empty()) continue;
            r.mountRoot = false;
            bool lastComp = p >= origRest.size();
            if (!missing) {
                if (comp.find_first_of("*?") != std::string::npos) { r.status = ST_OBJECT_NAME_INVALID; return r; }
                WIN32_FIND_DATAA fd;
                HANDLE h = FindFirstFileA((cur + "\\" + comp).c_str(), &fd);
                if (h != INVALID_HANDLE_VALUE) {
                    FindClose(h);
                    cur += "\\";
                    cur += fd.cFileName;
                    r.isDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                    if (!lastComp && !r.isDir) { r.exists = false; missing = true; r.status = ST_OBJECT_PATH_NOT_FOUND_; }
                    continue;
                }
                missing = true;
                r.exists = false;
                r.isDir = false;
                r.status = lastComp ? ST_OBJECT_NAME_NOT_FOUND_ : ST_OBJECT_PATH_NOT_FOUND_;
            }
            cur += "\\";
            cur += comp;
        }
        r.host = cur;
        if (missing) {
            // un componente intermedio ausente da PATH_NOT_FOUND; el ultimo, NAME_NOT_FOUND (el llamador decide si crea)
        }
    }
    return r;
}

} // namespace rt
