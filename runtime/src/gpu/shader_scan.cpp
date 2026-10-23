// Fase 4a: busca en la memoria guest contenedores de shader Xenos y los vuelca a runtime\userdata\shaders\xenos_<hash>.bin.
// Criterio de XenosRecomp (main.cpp, copia en runtime\ref\xenosrecomp_main.cpp): (flags & 0xFFFFFF00) == 0x102A1100,
// virtualSize + physicalSize <= bytes restantes, campos +0x1C y +0x20 a cero; ShaderContainer de shader.h (9 palabras big-endian).
#include <windows.h>

#include <cstdio>
#include <mutex>
#include <set>
#include <string>

#include "config.h"
#include "gpu.h"
#include "guest_memory.h"
#include "import_log.h"

namespace rt::gpu {

void ScanShaderContainers(const char* tag) {
    static std::mutex mu;
    static std::set<uint64_t> seen;
    std::lock_guard<std::mutex> lk(mu);
    uint8_t* base = GuestMemory::Base();
    uint64_t found = 0, fresh = 0, inImage = 0;
    std::string dir = GetPaths().userdataDir + "\\shaders";
    CreateDirectoryA(GetPaths().userdataDir.c_str(), nullptr);
    CreateDirectoryA(dir.c_str(), nullptr);
    uint64_t addr = 0;
    while (addr < 0x100000000ull) {
        MEMORY_BASIC_INFORMATION mi;
        if (!VirtualQuery(base + addr, &mi, sizeof(mi))) break;
        uint64_t rs = (uint64_t)mi.RegionSize;
        uint32_t g = (uint32_t)addr;
        bool skip = mi.State != MEM_COMMIT || (mi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) ||
                    (g >= kFuncTableBase && g < kFuncTableEnd) || (g >= kGpuRegBase && g < kGpuRegBase + kGpuRegSize) ||
                    (g >= kXmaRegBase && g < kXmaRegBase + kXmaRegSize);
        if (!skip && rs >= 36) {
            const uint8_t* p = base + addr;
            for (uint64_t i = 0; i + 36 <= rs; i += 4) {
                if (p[i] != 0x10 || p[i + 1] != 0x2A || p[i + 2] != 0x11) continue;
                auto rd = [&](uint32_t o) { return __builtin_bswap32(*reinterpret_cast<const uint32_t*>(p + i + o)); };
                uint64_t dsz = (uint64_t)rd(4) + rd(8);
                if (dsz < 36 || dsz > rs - i || rd(0x1C) != 0 || rd(0x20) != 0) continue;
                ++found;
                if (addr + i >= kImageBase && addr + i < kImageBase + kImageSize) ++inImage;
                uint64_t h = 1469598103934665603ull;
                for (uint64_t k = 0; k < dsz; ++k) { h ^= p[i + k]; h *= 1099511628211ull; }
                if (seen.insert(h).second) {
                    ++fresh;
                    char name[64];
                    std::snprintf(name, sizeof(name), "\\xenos_%016llX.bin", (unsigned long long)h);
                    if (FILE* f = std::fopen((dir + name).c_str(), "wb")) { std::fwrite(p + i, 1, (size_t)dsz, f); std::fclose(f); }
                }
                i += (dsz & ~3ull) - 4;
            }
        }
        addr += rs;
    }
    ImportLog::Get().Printf("SHADER-CONTAINERS [%s] encontrados=%llu (en la imagen=%llu) nuevos_volcados=%llu unicos_total=%zu", tag,
                            (unsigned long long)found, (unsigned long long)inImage, (unsigned long long)fresh, seen.size());
}

} // namespace rt::gpu
