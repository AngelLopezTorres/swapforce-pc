#pragma once

#include <cstdint>
#include <string>

#include "ppc_config.h"
#include "ppc_context.h"

namespace rt::hid {

struct PortalProbeResult {
    bool found = false;
    bool opened = false;
    bool interfaceZero = false;
    bool interruptInFound = false;
    bool readAttempted = false;
    bool readSucceeded = false;
    bool readTimedOut = false;
    uint32_t error = 0;
    uint32_t bytesRead = 0;
    uint8_t endpoint = 0;
    std::string detail;
};

bool ProbePortal(PortalProbeResult& result);
bool PortalProbeAccepted(const PortalProbeResult& result);

void XamInputNonControllerGetRaw(PPCContext& ctx, uint8_t* base);
void XamInputNonControllerSetRaw(PPCContext& ctx, uint8_t* base);

} // namespace rt::hid
