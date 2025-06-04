#ifndef DPIHELPER_H
#define DPIHELPER_H

#include <vector>
#include <cstdint>

// Basic utilities for working with display DPI.
namespace DpiHelper {
    // Returns true on success.
    bool SetDPIScaling(uint32_t scalePercent, unsigned int monitorIndex = 0);
}

#endif // DPIHELPER_H
