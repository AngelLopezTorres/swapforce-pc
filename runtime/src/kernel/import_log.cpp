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
uint64_t ImportLog::Total() const { return impl_->total.load(); }
uint64_t ImportLog::Unique() const { return impl_->counts.size(); }

void ImportLog::Printf(const char* fmt, ...) {
    char buf[2048];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    std::lock_guard<std::mutex> g(impl_->mu);
    if (impl_->f) { std::fputs(buf, impl_->f); std::fputc('\n', impl_->f); std::fflush(impl_->f); }
    std::fputs(buf, stdout); std::fputc('\n', stdout);
}

void ImportLog::Enter(const char* name, bool impl, const PPCContext& ctx, uint8_t*) {
    g_lastImport = name;
    if (GuestThread* gt = GuestThread::Current()) gt->lastImport = name;
    uint64_t n;
    uint64_t seq = impl_->seq.fetch_add(1);
    impl_->total.fetch_add(1);
    bool full;
    {
        std::lock_guard<std::mutex> g(impl_->mu);
        n = ++impl_->counts[name];
        impl_->implFlag[name] = impl;
        full = cfg_.logAll || n <= cfg_.maxFullLogsPerName;
        if (full && impl_->f) {
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
    std::lock_guard<std::mutex> g(impl_->mu);
    uint64_t n = impl_->counts[name];
    if ((cfg_.logAll || n <= cfg_.maxFullLogsPerName) && impl_->f) {
        std::fprintf(impl_->f, "   tid=%lu %s -> 0x%08X\n", GetCurrentThreadId(), name, ctx.r3.u32);
        std::fflush(impl_->f);
    }
}

void ImportLog::Summary() {
    std::lock_guard<std::mutex> g(impl_->mu);
    if (impl_->f) {
        std::fprintf(impl_->f, "imports unique=%zu total=%llu\n", impl_->counts.size(),
                     (unsigned long long)impl_->total.load());
        for (auto& kv : impl_->counts)
            std::fprintf(impl_->f, "  count %-40s %llu %s\n", kv.first.c_str(), (unsigned long long)kv.second,
                         impl_->implFlag[kv.first] ? "impl" : "stub");
        std::fflush(impl_->f);
    }
    std::printf("imports unique=%zu total=%llu\n", impl_->counts.size(), (unsigned long long)impl_->total.load());
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
