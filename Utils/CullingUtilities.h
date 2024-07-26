#pragma once

#include <cstdint>
#include <cstddef>

namespace culling {

inline uint64_t calculateChecksum(const void* data, size_t size)
{
    std::cout << "Entering calculateChecksum" << std::endl;
    std::cout << "Data pointer: " << data << std::endl;
    std::cout << "Size: " << size << std::endl;

    if (data == nullptr) {
        std::cerr << "Error: Null pointer passed to calculateChecksum" << std::endl;
        return 0;
    }

    if (size == 0) {
        std::cerr << "Warning: Zero size passed to calculateChecksum" << std::endl;
        return 0;
    }

    uint64_t checksum = 0;
    const unsigned char* byteData = static_cast<const unsigned char*>(data);

    try {
        for (size_t i = 0; i < size; ++i) {
            if (i % 1000 == 0) {  // Print progress every 1000 bytes
                std::cout << "Processing byte " << i << " of " << size << std::endl;
            }
            checksum += byteData[i];
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught in calculateChecksum: " << e.what() << std::endl;
        return 0;
    }
    catch (...) {
        std::cerr << "Unknown exception caught in calculateChecksum" << std::endl;
        return 0;
    }

    std::cout << "Exiting calculateChecksum. Checksum: " << checksum << std::endl;
    return checksum;
}

} // namespace culling