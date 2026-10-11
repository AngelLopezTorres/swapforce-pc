#pragma once
// Asignador de rangos sobre una region fija del espacio guest (reserva+confirmacion en bloque).
#include <cstdint>
#include <map>
#include <mutex>

namespace rt {

class GuestHeap {
public:
    GuestHeap(uint32_t begin, uint32_t endExclusive, uint32_t granularity);
    uint32_t Alloc(uint32_t size, uint32_t align = 0, bool commit = true);     // 0 en fallo; memoria a cero
    uint32_t AllocAt(uint32_t addr, uint32_t size, bool commit = true);         // 0 en fallo (solapa o fuera de rango)
    bool Free(uint32_t addr, uint32_t* sizeOut = nullptr);  // addr debe ser el inicio del bloque
    uint32_t BlockSize(uint32_t addr);                      // 0 si no es inicio de bloque
    bool Contains(uint32_t addr) const { return addr >= begin_ && addr < end_; }
    // Bloque que contiene addr: devuelve inicio y tamano (false si no hay)
    bool Find(uint32_t addr, uint32_t* start, uint32_t* size);
    uint32_t Begin() const { return begin_; }
    uint32_t End() const { return end_; }
    uint32_t BytesInUse();

private:
    uint32_t begin_, end_, gran_;
    std::map<uint32_t, uint32_t> blocks_;
    std::mutex mu_;
};

// Regiones del mapa guest (ninguna solapa 0x82000000-0x84E43DE8).
GuestHeap& SystemHeap();   // 0x86000000-0x90000000: PCR, TLS, KTHREAD, pools del kernel
GuestHeap& StackHeap();    // 0x70000000-0x7F000000: pilas de hilos guest
GuestHeap& VirtualHeap();  // 0x40000000-0x70000000: NtAllocateVirtualMemory
GuestHeap& PhysicalHeap(); // 0xA0000000-0xC0000000: MmAllocatePhysicalMemoryEx

} // namespace rt
