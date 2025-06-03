#pragma once
#include <Windows.h>
#include <vector>
#include "DpiHelper.h"

struct DisplayData {
    LUID m_adapterId;
    int m_targetID;
    int m_sourceID;
    DisplayData();
};

std::vector<DisplayData> GetDisplayData();
bool DpiValueSupported(int val);
