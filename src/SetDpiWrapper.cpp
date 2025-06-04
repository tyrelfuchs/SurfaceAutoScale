#include "DpiHelper.h"
#include <Windows.h>
#include <vector>

struct DisplayData {
    LUID adapterId;
    int targetID;
    int sourceID;
    DisplayData() { adapterId = {}; targetID = sourceID = -1; }
};

static std::vector<DisplayData> GetDisplayData()
{
    std::vector<DisplayData> displayDataCache;
    std::vector<DISPLAYCONFIG_PATH_INFO> pathsV;
    std::vector<DISPLAYCONFIG_MODE_INFO> modesV;
    int flags = QDC_ONLY_ACTIVE_PATHS;
    if (!DpiHelper::GetPathsAndModes(pathsV, modesV, flags))
    {
        return displayDataCache;
    }
    displayDataCache.resize(pathsV.size());
    int idx = 0;
    for (const auto& path : pathsV)
    {
        auto adapterLUID = path.targetInfo.adapterId;
        auto targetID = path.targetInfo.id;
        auto sourceID = path.sourceInfo.id;

        DisplayData dd{};
        dd.adapterId = adapterLUID;
        dd.sourceID = sourceID;
        dd.targetID = targetID;

        displayDataCache[idx] = dd;
        idx += 1;
    }
    return displayDataCache;
}

void SetDpiForPrimaryDisplay(int dpiValue)
{
    auto displays = GetDisplayData();
    if (displays.empty())
        return;
    DpiHelper::SetDPIScaling(displays[0].adapterId, displays[0].sourceID, dpiValue);
}
