#include <corecrt_wstdio.h>
#define UNICODE
#define _UNICODE
#include <Windows.h>
#include <shellapi.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_ENABLE 1001
#define ID_TRAY_DISABLE 1002
#define ID_TRAY_EXIT 1003

// Global variables
HINSTANCE hInst;
NOTIFYICONDATA nid;
bool isEnabled = true;  // Control whether the program is active

bool isTabletMode() {
    return GetSystemMetrics(SM_CONVERTIBLESLATEMODE) == 0;
}

int getNumberOfMonitors() {
    return GetSystemMetrics(SM_CMONITORS);
}

void setScaling(int dpiValue) {
    // Construct the command string with the desired DPI value
    wchar_t command[256];
    swprintf_s(command, L"C:\\AutoScale\\SetDpi.exe");
    wchar_t parameters[256];
    swprintf_s(parameters, L"1 %d", dpiValue);

    // Execute the command
    ShellExecuteW(NULL, L"open", command, parameters, NULL, SW_HIDE);
}

// Window Procedure to handle messages
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        // Initialize NOTIFYICONDATA structure
        ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uID = 1;  // Unique identifier
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_TRAYICON;
        nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wcscpy_s(nid.szTip, ARRAYSIZE(nid.szTip), L"AutoScale");

        // Add the icon to the system tray
        Shell_NotifyIcon(NIM_ADD, &nid);
        break;
    }
    case WM_TRAYICON: {
        if (lParam == WM_RBUTTONUP) {
            // Show context menu
            HMENU hMenu = CreatePopupMenu();
            if (isEnabled) {
                AppendMenu(hMenu, MF_STRING, ID_TRAY_DISABLE, L"Disable");
            } else {
                AppendMenu(hMenu, MF_STRING, ID_TRAY_ENABLE, L"Enable");
            }
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");

            // Get cursor position for the menu
            POINT pt;
            GetCursorPos(&pt);

            // Make the window foreground before showing the menu
            SetForegroundWindow(hwnd);

            // Display the menu
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
        }
        break;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_TRAY_ENABLE:
            isEnabled = true;
            MessageBox(hwnd, L"AutoScale Enabled", L"Status", MB_OK);
            break;
        case ID_TRAY_DISABLE:
            isEnabled = false;
            MessageBox(hwnd, L"AutoScale Disabled", L"Status", MB_OK);
            break;
        case ID_TRAY_EXIT:
            // Remove the tray icon and exit
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
        }
        break;
    }
    case WM_DESTROY: {
        // Clean up the tray icon
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    hInst = hInstance;

    // Register window class
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"AutoScaleClass";
    RegisterClass(&wc);

    // Create an invisible window
    HWND hwnd = CreateWindow(L"AutoScaleClass", L"AutoScale", WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
                             NULL, NULL, hInst, NULL);

    // Main message loop
    MSG msg;
    bool currentMode = isTabletMode();
    bool lastMode = currentMode;
    int currentMonitorCount = getNumberOfMonitors();
    int lastMonitorCount = currentMonitorCount;
    int lastScaling = 0;

    while (true) {
        // Handle Windows messages
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                return 0;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Check monitor count and tablet mode if enabled
        if (isEnabled) {
            currentMonitorCount = getNumberOfMonitors();
            currentMode = isTabletMode();

            // Determine desired scaling based on monitor count and tablet mode
            int desiredScaling = 0;
            if (currentMonitorCount > 1) {
                desiredScaling = 175; // More than one monitor
            } else {
                if (currentMode) {
                    desiredScaling = 200; // Tablet mode with one monitor
                } else {
                    desiredScaling = 175; // Desktop mode with one monitor
                }
            }

            // Apply scaling if it has changed
            if (desiredScaling != lastScaling) {
                setScaling(desiredScaling);
                lastScaling = desiredScaling;
            }

            // Update last known states
            lastMonitorCount = currentMonitorCount;
            lastMode = currentMode;
        }

        Sleep(5000);  // Check every 5 seconds
    }

    return 0;
}
