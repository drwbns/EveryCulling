#pragma once

#include <cstddef>
#include <iostream>

#define NOMINMAX
#include <Windows.h>


namespace MemoryUtils {

inline void ProtectMemory(void* addr, size_t len) {
    DWORD oldProtect;
    if (!VirtualProtect(addr, len, PAGE_READONLY, &oldProtect)) {
        std::cerr << "Failed to protect memory. Error code: " << GetLastError() << std::endl;
    }
}

inline void UnprotectMemory(void* addr, size_t len) {
    DWORD oldProtect;
    if (!VirtualProtect(addr, len, PAGE_READWRITE, &oldProtect)) {
        std::cerr << "Failed to unprotect memory. Error code: " << GetLastError() << std::endl;
    }
}

} // namespace MemoryUtils