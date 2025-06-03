#include "DpiUtils.h"
#include <iostream>

DisplayData::DisplayData() {
    m_adapterId = {};
    m_targetID = m_sourceID = -1;
}

std::vector<DisplayData> GetDisplayData() {
    std::vector<DisplayData> displayDataCache;
    std::vector<DISPLAYCONFIG_PATH_INFO> pathsV;
    std::vector<DISPLAYCONFIG_MODE_INFO> modesV;
    int flags = QDC_ONLY_ACTIVE_PATHS;
    if (!DpiHelper::GetPathsAndModes(pathsV, modesV, flags)) {
        std::cout << "DpiHelper::GetPathsAndModes() failed\n";
    }
    displayDataCache.resize(pathsV.size());
    for (int idx = 0; const auto &path : pathsV) {
        auto adapterLUID = path.targetInfo.adapterId;
        auto targetID = path.targetInfo.id;
        auto sourceID = path.sourceInfo.id;

        DISPLAYCONFIG_TARGET_DEVICE_NAME deviceName;
        deviceName.header.size = sizeof(deviceName);
        deviceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        deviceName.header.adapterId = adapterLUID;
        deviceName.header.id = targetID;
        if (ERROR_SUCCESS != DisplayConfigGetDeviceInfo(&deviceName.header)) {
            std::cout << "DisplayConfigGetDeviceInfo() failed";
        } else {
            DisplayData dd;
            dd.m_adapterId = adapterLUID;
            dd.m_sourceID = sourceID;
            dd.m_targetID = targetID;
            displayDataCache[idx] = dd;
        }
        idx += 1;
    }
    return displayDataCache;
}

bool DpiValueSupported(int val) {
    for (int i = 0; i < 12; i++) {
        if (val == (int)DpiVals[i]) {
            return true;
        }
    }
    return false;
}
