#include "import_log.h"

#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <mutex>
#include <thread>

#include <windows.h>

#include "guest_thread.h"

namespace rt {

struct ImportLog::Impl {
    std::mutex mu;
    FILE* f = nullptr;
    std::map<std::string, uint64_t> counts;
    std::map<std::string, bool> implFlag;
    std::atomic<uint64_t> seq{0};
    std::atomic<uint64_t> total{0};
};

static thread_local const char* g_lastImport = "";
bool g_importSerialize = false;   // M8: --import-serialize
static std::mutex g_serMu;

// M8: contadores por nombre sin bloqueo. Clave = puntero del literal del nombre (un mismo nombre puede tener varios punteros:
// el resumen agrega por texto). Tabla de direccionamiento abierto; la insercion es por CAS.
namespace {
struct NameSlot {
    std::atomic<const char*> key{nullptr};
    std::atomic<uint64_t> n{0};
    std::atomic<bool> impl{false};
};
constexpr size_t kSlots = 8192;
NameSlot g_slots[kSlots];
NameSlot* SlotFor(const char* name) {
    size_t h = (size_t)(((uintptr_t)name >> 3) * 0x9E3779B97F4A7C15ull) & (kSlots - 1);
    for (size_t i = 0; i < kSlots; ++i) {
        NameSlot& s = g_slots[(h + i) & (kSlots - 1)];
        const char* k = s.key.load(std::memory_order_acquire);
        if (k == name) return &s;
        if (!k) {
            const char* z = nullptr;
            if (s.key.compare_exchange_strong(z, name)) return &s;
            if (z == name) return &s;
        }
    }
    return nullptr;
}
} // namespace
static thread_local uint32_t g_guestAddr = 0;

ImportLog& ImportLog::Get() {
    static ImportLog inst;
    if (!inst.impl_) inst.impl_ = new Impl();
    return inst;
}

bool ImportLog::Open(const std::string& path) {
    std::lock_guard<std::mutex> g(impl_->mu);
    if (impl_->f) return true;
    impl_->f = std::fopen(path.c_str(), "w");
    return impl_->f != nullptr;
}

void ImportLog::Close() {
    std::lock_guard<std::mutex> g(impl_->mu);
    if (impl_->f) { std::fclose(impl_->f); impl_->f = nullptr; }
}

const char* ImportLog::LastImport() { return g_lastImport; }
void ImportLog::SetGuestAddr(uint32_t a) { g_guestAddr = a; }
uint32_t ImportLog::GuestAddr() { return g_guestAddr; }
uint64_t ImportLog::Total() const {
    uint64_t t = 0;
    for (auto& s : g_slots) t += s.n.load(std::memory_order_relaxed);
    return t;
}
uint64_t ImportLog::Unique() const {
    std::map<std::string, int> u;
    for (auto& s : g_slots) if (const char* k = s.key.load()) u[k] = 1;
    return u.size();
}

bool g_importLogTime = false;
bool g_m60Diag = false;   // M60: diagnosticos de tiempo del kernel (--gpu-timing)   // M60: --log-time (prefijo de marca de tiempo en ms desde el primer registro)

void ImportLog::Printf(const char* fmt, ...) {
    char buf[2048];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    char stamp[32] = "";
    if (g_importLogTime) {
        static LARGE_INTEGER freq{}, t0{};
        if (!t0.QuadPart) { QueryPerformanceFrequency(&freq); QueryPerformanceCounter(&t0); }
        LARGE_INTEGER now; QueryPerformanceCounter(&now);
        std::snprintf(stamp, sizeof(stamp), "[%.3f] ", (double)(now.QuadPart - t0.QuadPart) * 1000.0 / (double)freq.QuadPart);
    }
    std::lock_guard<std::mutex> g(impl_->mu);
    if (impl_->f) { std::fputs(stamp, impl_->f); std::fputs(buf, impl_->f); std::fputc('\n', impl_->f); std::fflush(impl_->f); }
    std::fputs(stamp, stdout); std::fputs(buf, stdout); std::fputc('\n', stdout);
}

void ImportLog::Enter(const char* name, bool impl, const PPCContext& ctx, uint8_t*) {
    g_lastImport = name;
    if (GuestThread* gt = GuestThread::Current()) gt->lastImport = name;
    if (g_importSerialize) { std::lock_guard<std::mutex> g(g_serMu); }   // M8 (diagnostico): serializacion como antes
    NameSlot* slot = SlotFor(name);
    uint64_t n = slot ? slot->n.fetch_add(1, std::memory_order_relaxed) + 1 : 1;
    if (slot && !slot->impl.load(std::memory_order_relaxed) && impl) slot->impl.store(true, std::memory_order_relaxed);
    bool full = cfg_.logAll || n <= cfg_.maxFullLogsPerName;
    uint64_t seq = (full || cfg_.maxImports) ? impl_->seq.fetch_add(1) : 0;   // numero de secuencia solo cuando se usa
    if (full) {
        std::lock_guard<std::mutex> g(impl_->mu);
        if (impl_->f) {
            std::fprintf(impl_->f,
                "#%llu tid=%lu import %s %s r3=%08X r4=%08X r5=%08X r6=%08X lr=%08X n=%llu\n",
                (unsigned long long)seq, GetCurrentThreadId(), name, impl ? "impl" : "stub",
                ctx.r3.u32, ctx.r4.u32, ctx.r5.u32, ctx.r6.u32, (uint32_t)ctx.lr, (unsigned long long)n);
            std::fflush(impl_->f);
        }
    }
    if (cfg_.maxImports && seq + 1 >= cfg_.maxImports) {
        Summary();
        std::fflush(nullptr);
        ExitProcess(3);
    }
}

void ImportLog::Leave(const char* name, bool, const PPCContext& ctx) {
    NameSlot* slot = SlotFor(name);
    uint64_t n = slot ? slot->n.load(std::memory_order_relaxed) : 0;
    if (!(cfg_.logAll || n <= cfg_.maxFullLogsPerName)) return;
    std::lock_guard<std::mutex> g(impl_->mu);
    if (impl_->f) {
        std::fprintf(impl_->f, "   tid=%lu %s -> 0x%08X\n", GetCurrentThreadId(), name, ctx.r3.u32);
        std::fflush(impl_->f);
    }
}

void ImportLog::Summary() {
    std::map<std::string, std::pair<uint64_t, bool>> agg;   // agregado por texto del nombre
    uint64_t total = 0;
    for (auto& s : g_slots) {
        const char* k = s.key.load();
        if (!k) continue;
        auto& e = agg[k];
        e.first += s.n.load(); e.second = e.second || s.impl.load();
        total += s.n.load();
    }
    std::lock_guard<std::mutex> g(impl_->mu);
    if (impl_->f) {
        std::fprintf(impl_->f, "imports unique=%zu total=%llu\n", agg.size(), (unsigned long long)total);
        for (auto& kv : agg)
            std::fprintf(impl_->f, "  count %-40s %llu %s\n", kv.first.c_str(), (unsigned long long)kv.second.first, kv.second.second ? "impl" : "stub");
        std::fflush(impl_->f);
    }
    std::printf("imports unique=%zu total=%llu\n", agg.size(), (unsigned long long)total);
}

uint32_t StubDefaultStatus(const char* name) {
    if (name[0] == 'N' && name[1] == 't' && !ImportLog::Get().Config().stubsReturnZeroForNt)
        return ST_NOT_IMPLEMENTED;
    return 0;
}

void ImportStubBody(PPCContext& ctx, uint8_t* base, const char* name) {
    ImportLog::Get().Enter(name, false, ctx, base);
    ctx.r3.u64 = StubDefaultStatus(name);
    ImportLog::Get().Leave(name, false, ctx);
}

} // namespace rt
