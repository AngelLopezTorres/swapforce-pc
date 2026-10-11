// E/S de ficheros: Nt{Create,Open,Read,Write}File, Query/SetInformation, directorios, volumen, enlaces simbolicos.
// Comportamiento y layouts big-endian: Xenia (xboxkrnl_io.cc, xboxkrnl_io_info.cc, info/file.h); NTSTATUS: Windows.
// Las lecturas y escrituras son siempre sincronas (nunca STATUS_PENDING).
#include <windows.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "guest_abi.h"
#include "guest_heap.h"
#include "import_log.h"
#include "kobject.h"
#include "vfs.h"

using namespace rt;

namespace {

constexpr uint32_t ST_ACCESS_DENIED = 0xC0000022;
constexpr uint32_t ST_INVALID_PARAMETER = 0xC000000D;
constexpr uint32_t ST_NAME_INVALID = 0xC0000033;
constexpr uint32_t ST_NO_SUCH_FILE = 0xC000000F;
constexpr uint32_t ST_FILE_IS_A_DIRECTORY = 0xC00000BA;
constexpr uint32_t ST_NOT_A_DIRECTORY = 0xC0000103;
constexpr uint32_t ST_MEDIA_WRITE_PROTECTED = 0xC00000A2;
constexpr uint32_t ST_INFO_LENGTH_MISMATCH = 0xC0000004;
constexpr uint32_t ST_INVALID_INFO_CLASS = 0xC0000003;
constexpr uint32_t ST_BUFFER_OVERFLOW = 0x80000005;
constexpr uint32_t ST_UNSUCCESSFUL = 0xC0000001;

// FILE_INFORMATION_CLASS (wdm.h / Xenia info/file.h)
enum : uint32_t {
    kFileDirectoryInformation = 1, kFileBasicInformation = 4, kFileStandardInformation = 5, kFileInternalInformation = 6,
    kFileNameInformation = 9, kFileRenameInformation = 10, kFileDispositionInformation = 13, kFilePositionInformation = 14,
    kFileAlignmentInformation = 17, kFileAllocationInformation = 19, kFileEndOfFileInformation = 20,
    kFileNetworkOpenInformation = 34,
};
// FS_INFORMATION_CLASS
enum : uint32_t { kFileFsVolumeInformation = 1, kFileFsSizeInformation = 3, kFileFsDeviceInformation = 4, kFileFsAttributeInformation = 5 };

struct FileObj : KObject {
    FileObj() : KObject(KType::File) {}
    std::string guestPath, hostPath;
    bool isDir = false;
    bool readOnly = true;
    uint64_t pos = 0;
    // enumeracion de directorio
    std::vector<WIN32_FIND_DATAA> entries;
    size_t entryIdx = 0;
    bool enumStarted = false;
    std::mutex mu;
};

std::string ReadAnsi(uint8_t* base, uint32_t ansiPtr) {
    if (!ansiPtr) return "";
    uint16_t len = G16(base, ansiPtr);
    uint32_t buf = G32(base, ansiPtr + 4);
    if (!buf) return "";
    return std::string((const char*)base + buf, len);
}

std::shared_ptr<FileObj> LookupFile(uint32_t h) {
    auto o = HandleTable::Get().Lookup(h);
    if (!o || o->type != KType::File) return nullptr;
    return std::static_pointer_cast<FileObj>(o);
}

void SetIosb(uint8_t* base, uint32_t iosb, uint32_t status, uint32_t info) {
    if (!iosb) return;
    P32(base, iosb, status);
    P32(base, iosb + 4, info);
}

bool ValidPath(const std::string& s, bool pattern) {
    for (char c : s) {
        unsigned char u = (unsigned char)c;
        if (u <= 31 || u >= 127) return false;
        switch (c) {
            case '"': case '+': case ',': case ';': case '<': case '=': case '>': case '|': return false;
            case '*': case '?': if (!pattern) return false; break;
            default: break;
        }
    }
    return true;
}

uint32_t XAttrs(DWORD a) {
    uint32_t r = a & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    return r ? r : FILE_ATTRIBUTE_NORMAL;
}

uint64_t FtU64(const FILETIME& f) { return ((uint64_t)f.dwHighDateTime << 32) | f.dwLowDateTime; }
uint64_t AllocSize(uint64_t sz) { return (sz + 0xFFF) & ~0xFFFull; }

// Rellena FILE_NETWORK_OPEN_INFORMATION (0x38 bytes, big-endian) desde WIN32_FILE_ATTRIBUTE_DATA.
void FillNetworkOpen(uint8_t* base, uint32_t p, const WIN32_FILE_ATTRIBUTE_DATA& d) {
    uint64_t size = ((uint64_t)d.nFileSizeHigh << 32) | d.nFileSizeLow;
    P64(base, p + 0x00, FtU64(d.ftCreationTime));
    P64(base, p + 0x08, FtU64(d.ftLastAccessTime));
    P64(base, p + 0x10, FtU64(d.ftLastWriteTime));
    P64(base, p + 0x18, FtU64(d.ftLastWriteTime));
    P64(base, p + 0x20, AllocSize(size));
    P64(base, p + 0x28, size);
    P32(base, p + 0x30, XAttrs(d.dwFileAttributes));
    P32(base, p + 0x34, 0);
}

bool ObjectName(uint8_t* base, uint32_t oa, std::shared_ptr<FileObj>* root, std::string* path, uint32_t* err) {
    if (!oa) { *err = ST_INVALID_PARAMETER; return false; }
    uint32_t rootH = G32(base, oa), namePtr = G32(base, oa + 4);
    *path = ReadAnsi(base, namePtr);
    if (!ValidPath(*path, false)) { *err = ST_NAME_INVALID; return false; }
    if (rootH != 0 && rootH != 0xFFFFFFFD) {
        *root = LookupFile(rootH);
        if (!*root) { *err = 0xC0000008; return false; }
        *path = (*root)->guestPath + "\\" + *path;
    }
    return true;
}

uint32_t DoCreate(PPCContext& ctx, uint8_t* base, uint32_t hOut, uint32_t access, uint32_t oa, uint32_t iosb, uint32_t disposition,
                  uint32_t options, const char* fn) {
    auto& log = ImportLog::Get();
    std::shared_ptr<FileObj> root;
    std::string path;
    uint32_t err = 0;
    if (!ObjectName(base, oa, &root, &path, &err)) {
        if (hOut) P32(base, hOut, 0xFFFFFFFF);
        SetIosb(base, iosb, err, 0);
        log.Printf("  %s ruta=\"%s\" -> 0x%08X (nombre no valido)", fn, path.c_str(), err);
        return err;
    }
    VfsResolved r = Vfs::Get().Resolve(path);
    uint32_t st = 0, info = 0;
    HANDLE hf = INVALID_HANDLE_VALUE;
    bool wantWrite = (access & (0x40000000 | 0x10000000 | 0x2 | 0x4)) != 0; // GENERIC_WRITE, GENERIC_ALL, FILE_WRITE_DATA, FILE_APPEND_DATA
    bool createsDir = (options & 1) != 0, nonDir = (options & 0x40) != 0;
    auto obj = std::make_shared<FileObj>();
    obj->guestPath = path;
    obj->readOnly = r.readOnly;

    if (r.status == ST_NAME_INVALID) st = r.status;
    else if (r.status == 0xC000003A && !(r.readOnly == false && false)) st = r.status; // ruta intermedia ausente
    else if (r.status != 0 && r.status != 0xC0000034) st = r.status;
    else {
        bool exists = r.exists;
        if (exists && r.isDir && nonDir) st = ST_FILE_IS_A_DIRECTORY;
        else if (exists && !r.isDir && createsDir) st = ST_NOT_A_DIRECTORY;
        else {
            bool create = false, trunc = false;
            switch (disposition) {
                case 0: create = !exists; trunc = exists; info = exists ? 0 : 2; break;           // SUPERSEDE
                case 1: if (!exists) st = 0xC0000034; info = 1; break;                           // OPEN
                case 2: if (exists) st = 0xC0000035; else create = true; info = 2; break;        // CREATE
                case 3: create = !exists; info = exists ? 1 : 2; break;                          // OPEN_IF
                case 4: if (!exists) st = 0xC0000034; else trunc = true; info = 3; break;        // OVERWRITE
                case 5: create = !exists; trunc = exists; info = exists ? 3 : 2; break;          // OVERWRITE_IF
                default: st = ST_INVALID_PARAMETER;
            }
            if (!st && (create || trunc || (exists && !r.isDir && wantWrite)) && r.readOnly) {
                st = (create || trunc) ? ST_MEDIA_WRITE_PROTECTED : ST_ACCESS_DENIED;
            }
            if (!st) {
                if (create && createsDir) {
                    if (!CreateDirectoryA(r.host.c_str(), nullptr)) st = 0xC000003A;
                    else { r.isDir = true; r.exists = true; }
                }
                DWORD acc = wantWrite ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
                DWORD share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
                if (!st) {
                    if (r.isDir || (create && createsDir)) {
                        hf = CreateFileA(r.host.c_str(), FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES, share, nullptr, OPEN_EXISTING,
                                         FILE_FLAG_BACKUP_SEMANTICS, nullptr);
                        obj->isDir = true;
                    } else {
                        DWORD cd = create ? (trunc ? CREATE_ALWAYS : CREATE_NEW) : (trunc ? TRUNCATE_EXISTING : OPEN_EXISTING);
                        DWORD fl = FILE_ATTRIBUTE_NORMAL | ((options & 0x800) ? FILE_FLAG_RANDOM_ACCESS : 0);
                        hf = CreateFileA(r.host.c_str(), acc, share, nullptr, cd, fl, nullptr);
                    }
                    if (hf == INVALID_HANDLE_VALUE) {
                        DWORD e = GetLastError();
                        st = (e == ERROR_FILE_NOT_FOUND) ? 0xC0000034 : (e == ERROR_PATH_NOT_FOUND) ? 0xC000003A
                           : (e == ERROR_ACCESS_DENIED) ? ST_ACCESS_DENIED : (e == ERROR_SHARING_VIOLATION) ? 0xC0000043 : ST_UNSUCCESSFUL;
                    }
                }
            }
        }
    }
    uint32_t handle = 0xFFFFFFFF;
    if (!st) {
        obj->host = hf;
        obj->ownsHost = true;
        obj->hostPath = r.host;
        handle = HandleTable::Get().Add(obj);
    } else {
        info = 0;
    }
    if (hOut) P32(base, hOut, handle);
    SetIosb(base, iosb, st, info);
    log.Printf("  %s ruta=\"%s\" resuelta=\"%s\" access=%08X disp=%u opts=%X -> 0x%08X handle=%08X info=%u", fn, path.c_str(), r.host.c_str(),
               access, disposition, options, st, handle, info);
    return st;
}

} // namespace

GUEST_IMPORT_IMPL(NtCreateFile) {
    ImportScope sc("NtCreateFile", ctx, base);
    // (HANDLE* out, access, OBJECT_ATTRIBUTES*, IO_STATUS_BLOCK*, alloc size*, attrs, share, disposition, [r1+0x54] options)
    SetRet(ctx, DoCreate(ctx, base, ctx.r3.u32, ctx.r4.u32, ctx.r5.u32, ctx.r6.u32, ctx.r10.u32, Arg32(ctx, base, 8), "NtCreateFile"));
}

GUEST_IMPORT_IMPL(NtOpenFile) {
    ImportScope sc("NtOpenFile", ctx, base);
    // (HANDLE* out, access, OBJECT_ATTRIBUTES*, IO_STATUS_BLOCK*, open options)
    SetRet(ctx, DoCreate(ctx, base, ctx.r3.u32, ctx.r4.u32, ctx.r5.u32, ctx.r6.u32, 1, ctx.r7.u32, "NtOpenFile"));
}

static uint32_t DoRead(PPCContext& ctx, uint8_t* base, bool write) {
    // (file, event, apc routine, apc context, IOSB*, buffer, length, byte offset*)
    uint32_t hf = ctx.r3.u32, hev = ctx.r4.u32, iosb = ctx.r7.u32, buf = ctx.r8.u32, len = ctx.r9.u32, offPtr = ctx.r10.u32;
    auto f = LookupFile(hf);
    uint32_t st = ST_SUCCESS, done = 0;
    if (!f) st = 0xC0000008;
    else if (f->isDir) st = ST_INVALID_PARAMETER;
    else if (write && f->readOnly) st = ST_MEDIA_WRITE_PROTECTED;
    else {
        std::lock_guard<std::mutex> g(f->mu);
        uint64_t off = f->pos;
        if (offPtr) {
            uint64_t o = G64(base, offPtr);
            if (o != 0xFFFFFFFFFFFFFFFFull) off = o;
        }
        OVERLAPPED ov = {};
        ov.Offset = (DWORD)off;
        ov.OffsetHigh = (DWORD)(off >> 32);
        DWORD n = 0;
        BOOL ok = write ? WriteFile(f->host, base + buf, len, &n, &ov) : ReadFile(f->host, base + buf, len, &n, &ov);
        if (!ok) {
            DWORD e = GetLastError();
            if (!write && e == ERROR_HANDLE_EOF) { n = 0; st = ST_END_OF_FILE; }
            else st = ST_UNSUCCESSFUL;
        } else if (!write && n == 0 && len != 0) st = ST_END_OF_FILE;
        done = n;
        f->pos = off + n;
    }
    SetIosb(base, iosb, st, done);
    if (hev) {
        auto ev = HandleTable::Get().Lookup(hev);
        if (ev && ev->type == KType::Event) SetEvent(ev->host);
    }
    ImportLog::Get().Printf("  %s handle=%08X len=%u off=%s bytes=%u -> 0x%08X", write ? "NtWriteFile" : "NtReadFile", hf, len,
                            offPtr ? "explicito" : "actual", done, st);
    return st;
}

GUEST_IMPORT_IMPL(NtReadFile) {
    ImportScope sc("NtReadFile", ctx, base);
    SetRet(ctx, DoRead(ctx, base, false));
}

GUEST_IMPORT_IMPL(NtWriteFile) {
    ImportScope sc("NtWriteFile", ctx, base);
    SetRet(ctx, DoRead(ctx, base, true));
}

GUEST_IMPORT_IMPL(NtReadFileScatter) {
    ImportScope sc("NtReadFileScatter", ctx, base);
    // (file, event, apc, apc ctx, IOSB*, segment array*, length, byte offset*): segmentos de 8 bytes cuyo primer be32 es el puntero [NV]
    uint32_t hf = ctx.r3.u32, iosb = ctx.r7.u32, segs = ctx.r8.u32, len = ctx.r9.u32, offPtr = ctx.r10.u32;
    auto f = LookupFile(hf);
    uint32_t st = ST_SUCCESS, total = 0;
    if (!f || f->isDir) st = 0xC0000008;
    else {
        std::lock_guard<std::mutex> g(f->mu);
        uint64_t off = f->pos;
        if (offPtr) { uint64_t o = G64(base, offPtr); if (o != 0xFFFFFFFFFFFFFFFFull) off = o; }
        uint32_t remaining = len;
        for (uint32_t i = 0; remaining; ++i) {
            uint32_t seg = G32(base, segs + i * 8 + 4); // be64: puntero guest en la mitad baja [NV]
            if (!seg) break;
            uint32_t n = remaining < 0x1000 ? remaining : 0x1000; // FILE_SEGMENT_ELEMENT = pagina de 4 KB
            OVERLAPPED ov = {};
            ov.Offset = (DWORD)off; ov.OffsetHigh = (DWORD)(off >> 32);
            DWORD got = 0;
            if (!ReadFile(f->host, base + seg, n, &got, &ov) || got == 0) break;
            off += got; total += got; remaining -= got;
            if (got < n) break;
        }
        f->pos = off;
        if (total == 0 && len) st = ST_END_OF_FILE;
    }
    SetIosb(base, iosb, st, total);
    ImportLog::Get().Printf("  NtReadFileScatter handle=%08X len=%u bytes=%u -> 0x%08X", hf, len, total, st);
    SetRet(ctx, st);
}

GUEST_IMPORT_IMPL(NtFlushBuffersFile) {
    ImportScope sc("NtFlushBuffersFile", ctx, base);
    SetIosb(base, ctx.r4.u32, ST_SUCCESS, 0);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(NtQueryFullAttributesFile) {
    ImportScope sc("NtQueryFullAttributesFile", ctx, base);
    std::shared_ptr<FileObj> root;
    std::string path;
    uint32_t err = 0;
    if (!ObjectName(base, ctx.r3.u32, &root, &path, &err)) { SetRet(ctx, err); return; }
    VfsResolved r = Vfs::Get().Resolve(path);
    uint32_t st = ST_NO_SUCH_FILE;
    WIN32_FILE_ATTRIBUTE_DATA d;
    if (r.status == 0 && GetFileAttributesExA(r.host.c_str(), GetFileExInfoStandard, &d)) {
        FillNetworkOpen(base, ctx.r4.u32, d);
        st = ST_SUCCESS;
    } else if (r.status == ST_NAME_INVALID) st = r.status;
    ImportLog::Get().Printf("  NtQueryFullAttributesFile ruta=\"%s\" resuelta=\"%s\" -> 0x%08X", path.c_str(), r.host.c_str(), st);
    SetRet(ctx, st);
}

GUEST_IMPORT_IMPL(NtQueryInformationFile) {
    ImportScope sc("NtQueryInformationFile", ctx, base);
    // (handle, IOSB*, buffer, length, class)
    uint32_t h = ctx.r3.u32, iosb = ctx.r4.u32, buf = ctx.r5.u32, len = ctx.r6.u32, cls = ctx.r7.u32;
    auto f = LookupFile(h);
    uint32_t st = ST_SUCCESS, outLen = 0;
    uint32_t minLen = cls == kFileInternalInformation ? 8 : cls == kFilePositionInformation ? 8 : cls == kFileNetworkOpenInformation ? 56
                    : cls == kFileAlignmentInformation ? 4 : 0;
    if (!f) st = 0xC0000008;
    else if (!minLen) st = ST_INVALID_INFO_CLASS;
    else if (len < minLen) st = ST_INFO_LENGTH_MISMATCH;
    else {
        std::memset(base + buf, 0, len);
        switch (cls) {
            case kFileInternalInformation: {
                uint64_t hsh = 1469598103934665603ull;
                for (char c : f->guestPath) hsh = (hsh ^ (uint8_t)c) * 1099511628211ull;
                P64(base, buf, hsh); outLen = 8; break;
            }
            case kFilePositionInformation: P64(base, buf, f->pos); outLen = 8; break;
            case kFileNetworkOpenInformation: {
                WIN32_FILE_ATTRIBUTE_DATA d;
                if (GetFileAttributesExA(f->hostPath.c_str(), GetFileExInfoStandard, &d)) { FillNetworkOpen(base, buf, d); outLen = 56; }
                else st = ST_UNSUCCESSFUL;
                break;
            }
            case kFileAlignmentInformation: P32(base, buf, 0); outLen = 4; break;
        }
    }
    SetIosb(base, iosb, st, outLen);
    ImportLog::Get().Printf("  NtQueryInformationFile handle=%08X class=%u -> 0x%08X", h, cls, st);
    SetRet(ctx, st);
}

GUEST_IMPORT_IMPL(NtSetInformationFile) {
    ImportScope sc("NtSetInformationFile", ctx, base);
    uint32_t h = ctx.r3.u32, iosb = ctx.r4.u32, buf = ctx.r5.u32, len = ctx.r6.u32, cls = ctx.r7.u32;
    auto f = LookupFile(h);
    uint32_t st = ST_SUCCESS, outLen = 0;
    uint32_t minLen = cls == kFileDispositionInformation ? 1 : (cls == kFilePositionInformation || cls == kFileEndOfFileInformation || cls == kFileAllocationInformation) ? 8 : 0;
    if (!f) st = 0xC0000008;
    else if (!minLen) st = ST_INVALID_INFO_CLASS;
    else if (len < minLen) st = ST_INFO_LENGTH_MISMATCH;
    else {
        switch (cls) {
            case kFilePositionInformation: f->pos = G64(base, buf); outLen = 8; break;
            case kFileEndOfFileInformation: {
                if (f->readOnly) { st = ST_MEDIA_WRITE_PROTECTED; break; }
                LARGE_INTEGER li; li.QuadPart = (LONGLONG)G64(base, buf);
                if (!SetFilePointerEx(f->host, li, nullptr, FILE_BEGIN) || !SetEndOfFile(f->host)) st = ST_UNSUCCESSFUL;
                outLen = 8; break;
            }
            case kFileAllocationInformation: outLen = 8; break;
            case kFileDispositionInformation: break; // borrado al cerrar: ignorado (como Xenia)
        }
    }
    SetIosb(base, iosb, st, outLen);
    ImportLog::Get().Printf("  NtSetInformationFile handle=%08X class=%u -> 0x%08X", h, cls, st);
    SetRet(ctx, st);
}

GUEST_IMPORT_IMPL(NtQueryVolumeInformationFile) {
    ImportScope sc("NtQueryVolumeInformationFile", ctx, base);
    uint32_t h = ctx.r3.u32, iosb = ctx.r4.u32, buf = ctx.r5.u32, len = ctx.r6.u32, cls = ctx.r7.u32;
    auto f = LookupFile(h);
    uint32_t st = ST_SUCCESS, outLen = 0;
    if (!f) st = 0xC0000008;
    else switch (cls) {
        case kFileFsSizeInformation:
            if (len < 0x18) { st = ST_INFO_LENGTH_MISMATCH; break; }
            { // 512 B por sector; capacidad ficticia fija [NV]
                std::memset(base + buf, 0, len);
                P64(base, buf, 0x1000000); P64(base, buf + 8, 0x800000);
                P32(base, buf + 16, 1); P32(base, buf + 20, 0x200); outLen = 0x18;
            }
            break;
        case kFileFsAttributeInformation: {
            // {be32 atributos, be32 longitud maxima de componente, be32 longitud del nombre, char nombre[]} (Xenia)
            // Valores de Xenia (disc_image_device.h): atributos 0, componente 255; nombre "GDFX" (disco) / "FATX" [NV].
            const char* nm = f->readOnly ? "GDFX" : "FATX";
            uint32_t nl = 4;
            if (len < 12) { st = ST_INFO_LENGTH_MISMATCH; break; }
            std::memset(base + buf, 0, len);
            P32(base, buf, 0); P32(base, buf + 4, f->readOnly ? 255 : 40); P32(base, buf + 8, nl);
            if (len >= 12 + nl) { std::memcpy(base + buf + 12, nm, nl); outLen = 12 + nl; }
            else { st = ST_BUFFER_OVERFLOW; outLen = 12; }
            break;
        }
        default: st = ST_INVALID_INFO_CLASS; break;
    }
    SetIosb(base, iosb, st, outLen);
    ImportLog::Get().Printf("  NtQueryVolumeInformationFile handle=%08X class=%u -> 0x%08X", h, cls, st);
    SetRet(ctx, st);
}

GUEST_IMPORT_IMPL(NtQueryDirectoryFile) {
    ImportScope sc("NtQueryDirectoryFile", ctx, base);
    // (handle, event, apc, apc ctx, IOSB*, info*, length, FileName(ANSI_STRING*), [r1+0x54] restart)
    uint32_t h = ctx.r3.u32, iosb = ctx.r7.u32, info = ctx.r8.u32, len = ctx.r9.u32, namePtr = ctx.r10.u32;
    bool restart = Arg32(ctx, base, 8) != 0;
    auto f = LookupFile(h);
    uint32_t st = ST_UNSUCCESSFUL, outLen = 0;
    std::string pat = ReadAnsi(base, namePtr);
    if (len < 72) st = ST_INFO_LENGTH_MISMATCH;
    else if (!ValidPath(pat, true)) st = ST_INVALID_PARAMETER;
    else if (!f || !f->isDir) st = ST_NO_SUCH_FILE;
    else {
        std::lock_guard<std::mutex> g(f->mu);
        if (!pat.empty() || restart || !f->enumStarted) {
            f->entries.clear();
            f->entryIdx = 0;
            f->enumStarted = true;
            WIN32_FIND_DATAA fd;
            HANDLE fh = FindFirstFileA((f->hostPath + "\\" + (pat.empty() ? "*" : pat)).c_str(), &fd);
            if (fh != INVALID_HANDLE_VALUE) {
                do {
                    if (!std::strcmp(fd.cFileName, ".") || !std::strcmp(fd.cFileName, "..")) continue;
                    f->entries.push_back(fd);
                } while (FindNextFileA(fh, &fd));
                FindClose(fh);
            }
        }
        if (f->entryIdx >= f->entries.size()) st = pat.empty() ? ST_NO_MORE_FILES : ST_NO_SUCH_FILE;
        else {
            const WIN32_FIND_DATAA& e = f->entries[f->entryIdx++];
            uint32_t nlen = (uint32_t)std::strlen(e.cFileName);
            if (0x40 + nlen > len) { st = ST_BUFFER_OVERFLOW; }
            else {
                uint64_t size = ((uint64_t)e.nFileSizeHigh << 32) | e.nFileSizeLow;
                std::memset(base + info, 0, 0x40);
                P32(base, info + 0x00, 0);
                P32(base, info + 0x04, (uint32_t)f->entryIdx);
                P64(base, info + 0x08, FtU64(e.ftCreationTime));
                P64(base, info + 0x10, FtU64(e.ftLastAccessTime));
                P64(base, info + 0x18, FtU64(e.ftLastWriteTime));
                P64(base, info + 0x20, FtU64(e.ftLastWriteTime));
                P64(base, info + 0x28, size);
                P64(base, info + 0x30, AllocSize(size));
                P32(base, info + 0x38, XAttrs(e.dwFileAttributes));
                P32(base, info + 0x3C, nlen);
                std::memcpy(base + info + 0x40, e.cFileName, nlen);
                st = ST_SUCCESS;
                outLen = len; // Xenia: information = length
            }
        }
    }
    SetIosb(base, iosb, st, st == ST_SUCCESS ? outLen : 0);
    ImportLog::Get().Printf("  NtQueryDirectoryFile handle=%08X patron=\"%s\" restart=%d -> 0x%08X", h, pat.c_str(), (int)restart, st);
    SetRet(ctx, st);
}

// ---- Enlaces simbolicos ----
GUEST_IMPORT_IMPL(ObCreateSymbolicLink) {
    ImportScope sc("ObCreateSymbolicLink", ctx, base);
    std::string name = ReadAnsi(base, ctx.r3.u32), target = ReadAnsi(base, ctx.r4.u32);
    Vfs::Get().AddAlias(name, target);
    ImportLog::Get().Printf("  ObCreateSymbolicLink \"%s\" -> \"%s\"", name.c_str(), target.c_str());
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(ObDeleteSymbolicLink) {
    ImportScope sc("ObDeleteSymbolicLink", ctx, base);
    std::string name = ReadAnsi(base, ctx.r3.u32);
    bool ok = Vfs::Get().RemoveAlias(name);
    ImportLog::Get().Printf("  ObDeleteSymbolicLink \"%s\" -> %s", name.c_str(), ok ? "ok" : "no existia");
    SetRet(ctx, ok ? ST_SUCCESS : ST_NO_SUCH_FILE);
}

// ---- Dispositivos de almacenamiento (Stfs/Fsc/Io): exito + registro ----
GUEST_IMPORT_IMPL(FscSetCacheElementCount) { ImportScope sc("FscSetCacheElementCount", ctx, base); SetRet(ctx, ST_SUCCESS); }
GUEST_IMPORT_IMPL(StfsCreateDevice) { ImportScope sc("StfsCreateDevice", ctx, base); SetRet(ctx, ST_SUCCESS); }
GUEST_IMPORT_IMPL(StfsControlDevice) { ImportScope sc("StfsControlDevice", ctx, base); SetRet(ctx, ST_SUCCESS); }
GUEST_IMPORT_IMPL(IoDismountVolume) { ImportScope sc("IoDismountVolume", ctx, base); SetRet(ctx, ST_SUCCESS); }
GUEST_IMPORT_IMPL(IoDismountVolumeByFileHandle) { ImportScope sc("IoDismountVolumeByFileHandle", ctx, base); SetRet(ctx, ST_SUCCESS); }
