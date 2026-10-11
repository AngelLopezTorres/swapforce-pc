#pragma once
// Objetos del kernel (eventos, semaforos, mutantes, hilos, ficheros) y tabla de handles.
#include <windows.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace rt {

class GuestThread;

enum class KType { Event, Semaphore, Mutant, Thread, File, Other };

struct KObject {
    explicit KObject(KType t) : type(t) {}
    virtual ~KObject() {
        if (host && ownsHost) CloseHandle(host);
    }
    KType type;
    HANDLE host = nullptr;     // objeto host esperable (evento, semaforo, mutex, hilo)
    bool ownsHost = true;
    bool manualReset = false;  // eventos
    uint32_t nativePtr = 0;    // puntero guest (ObReferenceObjectByHandle / Ke*)
    GuestThread* thread = nullptr;
};

constexpr uint32_t kInvalidHandle = 0xFFFFFFFF;
constexpr uint32_t kCurrentThreadPseudoHandle = 0xFFFFFFFE;

class HandleTable {
public:
    static HandleTable& Get();
    uint32_t Add(std::shared_ptr<KObject> o);          // devuelve handle (>= 0xF8000000)
    std::shared_ptr<KObject> Lookup(uint32_t handle);
    bool Close(uint32_t handle);
    uint32_t Duplicate(uint32_t handle);               // 0 si invalido

    // Puntero guest nativo (bloque en el heap del sistema) asociado a un objeto.
    uint32_t NativePtr(const std::shared_ptr<KObject>& o);
    void SetNativePtr(const std::shared_ptr<KObject>& o, uint32_t ptr);
    std::shared_ptr<KObject> FromNative(uint32_t ptr);
    uint32_t OpenCount();

private:
    std::mutex mu_;
    std::unordered_map<uint32_t, std::shared_ptr<KObject>> handles_;
    std::unordered_map<uint32_t, std::shared_ptr<KObject>> native_;
    uint32_t next_ = 0xF8000004;
    uint32_t poolCur_ = 0, poolEnd_ = 0;
};

} // namespace rt
