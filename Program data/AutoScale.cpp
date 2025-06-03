#include <Windows.h>
#include <cstdlib>

bool isTabletMode() {
    return GetSystemMetrics(SM_CONVERTIBLESLATEMODE) == 0;
}

void setTabletScaling() {
    WinExec("C:\\AutoScale\\SetDpi.exe 1 200", SW_HIDE); // Set tablet mode to 200%
}

void setDesktopScaling() {
    WinExec("C:\\AutoScale\\SetDpi.exe 1 175", SW_HIDE); // Set desktop mode to 175%
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    bool currentMode, lastMode = isTabletMode();
    while (true) {
        currentMode = isTabletMode();
        if (currentMode && !lastMode) {
            setTabletScaling();
        } 
        else if (!currentMode && lastMode) {
            setDesktopScaling();
        }
        lastMode = currentMode;
        Sleep(5000); // Check every 5 seconds
    }

    return 0;
}
