#include "xex_header.h"

#include <cstdio>
#include <cstring>
#include <vector>

#include "guest_memory.h"

namespace rt {

bool LoadXexHeader(const std::string& xexPath, XexHeaderInfo& out, std::string* err) {
    FILE* f = std::fopen(xexPath.c_str(), "rb");
    if (!f) { if (err) *err = "no se puede abrir " + xexPath; return false; }
    uint8_t hdr[16];
    if (std::fread(hdr, 1, 16, f) != 16 || std::memcmp(hdr, "XEX2", 4) != 0) {
        std::fclose(f); if (err) *err = "firma XEX2 ausente"; return false;
    }
    auto be32 = [&](int o) { return (uint32_t)hdr[o] << 24 | hdr[o + 1] << 16 | hdr[o + 2] << 8 | hdr[o + 3]; };
    uint32_t dataOffset = be32(8);
    if (dataOffset < 16 || dataOffset > 0x10000) { std::fclose(f); if (err) *err = "data_offset invalido"; return false; }
    std::vector<uint8_t> buf(dataOffset);
    std::fseek(f, 0, SEEK_SET);
    size_t n = std::fread(buf.data(), 1, dataOffset, f);
    std::fclose(f);
    if (n != dataOffset) { if (err) *err = "cabecera truncada"; return false; }
    if (!GuestMemory::Commit(kXexHeaderGuest, (dataOffset + 0xFFF) & ~0xFFFu)) { if (err) *err = "commit cabecera"; return false; }
    std::memcpy(GuestMemory::Base() + kXexHeaderGuest, buf.data(), dataOffset);
    out.size = dataOffset;
    out.moduleFlags = be32(4);
    out.guestAddr = kXexHeaderGuest;
    return true;
}

} // namespace rt

namespace rt {

bool XexFindOptHeader(uint32_t key, uint32_t* value, uint32_t* guestStruct) {
    uint32_t count = GuestMemory::Read32(kXexHeaderGuest + 0x14);
    for (uint32_t i = 0; i < count && i < 0x100; ++i) {
        uint32_t k = GuestMemory::Read32(kXexHeaderGuest + 0x18 + i * 8);
        uint32_t v = GuestMemory::Read32(kXexHeaderGuest + 0x1C + i * 8);
        if (k != key) continue;
        if (value) *value = v;
        uint8_t low = k & 0xFF;
        if (guestStruct) *guestStruct = (low == 0 || low == 1) ? 0 : kXexHeaderGuest + v;
        return true;
    }
    return false;
}

XexTlsInfo XexGetTlsInfo() {
    XexTlsInfo t;
    uint32_t v, s;
    if (XexFindOptHeader(0x00020104, &v, &s) && s) {
        t.slotCount = GuestMemory::Read32(s + 0);
        t.rawDataAddress = GuestMemory::Read32(s + 4);
        t.dataSize = GuestMemory::Read32(s + 8);
        t.rawDataSize = GuestMemory::Read32(s + 12);
        t.present = true;
    }
    return t;
}

} // namespace rt
