#define UNICODE
#define _UNICODE
#include <Windows.h>
#include <shellapi.h>
#include <wchar.h>   // Include this header for swprintf_s

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "Advapi32.lib")  // Link against Advapi32.lib

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_ENABLE 1001
#define ID_TRAY_DISABLE 1002
#define ID_TRAY_AUTOSTART 1005
#define ID_TRAY_EXIT 1003
#define IDT_TIMER1 2001  // Timer ID

// Global variables
HINSTANCE hInst;
NOTIFYICONDATA nid;
bool isEnabled = true;         // Control whether the program is active
bool isAutoStartEnabled = false; // Control whether the app starts with Windows

bool isTabletMode() {
    return GetSystemMetrics(SM_CONVERTIBLESLATEMODE) == 0;
}

int getNumberOfMonitors() {
    return GetSystemMetrics(SM_CMONITORS);
}

void setScaling(int dpiValue) {
    // Construct the command string with the desired DPI value
    wchar_t command[256];
    swprintf_s(command, sizeof(command) / sizeof(wchar_t), L"C:\\AutoScale\\SetDpi.exe");
    wchar_t parameters[256];
    swprintf_s(parameters, sizeof(parameters) / sizeof(wchar_t), L"1 %d", dpiValue);

    // Execute the command
    ShellExecuteW(NULL, L"open", command, parameters, NULL, SW_HIDE);
}

// Variables to track the last known state
int lastScaling = 0;

void CheckAndApplyScaling() {
    if (isEnabled) {
        int currentMonitorCount = getNumberOfMonitors();
        bool currentMode = isTabletMode();

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

            // Optionally, update the tooltip or tray icon here
        }
    }
}

void EnableAutoStart(bool enable) {
    HKEY hKey;
    const wchar_t* runKeyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const wchar_t* valueName = L"AutoScale";

    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, runKeyPath, 0, KEY_WRITE, &hKey);
    if (result == ERROR_SUCCESS) {
        if (enable) {
            wchar_t path[MAX_PATH];
            GetModuleFileNameW(NULL, path, MAX_PATH);

            result = RegSetValueExW(hKey, valueName, 0, REG_SZ, (BYTE*)path, (wcslen(path) + 1) * sizeof(wchar_t));
            if (result != ERROR_SUCCESS) {
                MessageBoxW(NULL, L"Failed to enable auto-start.", L"Error", MB_OK | MB_ICONERROR);
            }
        } else {
            result = RegDeleteValueW(hKey, valueName);
            if (result != ERROR_SUCCESS && result != ERROR_FILE_NOT_FOUND) {
                MessageBoxW(NULL, L"Failed to disable auto-start.", L"Error", MB_OK | MB_ICONERROR);
            }
        }
        RegCloseKey(hKey);
    } else {
        MessageBoxW(NULL, L"Failed to access registry for auto-start.", L"Error", MB_OK | MB_ICONERROR);
    }
}

bool IsAutoStartEnabled() {
    HKEY hKey;
    const wchar_t* runKeyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const wchar_t* valueName = L"AutoScale";
    bool enabled = false;

    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, runKeyPath, 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        DWORD valueType = 0;
        wchar_t valueData[MAX_PATH];
        DWORD valueSize = sizeof(valueData);

        result = RegQueryValueExW(hKey, valueName, NULL, &valueType, (LPBYTE)valueData, &valueSize);
        if (result == ERROR_SUCCESS && valueType == REG_SZ) {
            enabled = true;
        }
        RegCloseKey(hKey);
    }
    return enabled;
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

        // Set a timer to check scaling every 5 seconds
        SetTimer(hwnd, IDT_TIMER1, 5000, NULL);
        break;
    }
    case WM_TIMER: {
        if (wParam == IDT_TIMER1) {
            // Timer event occurred, check and apply scaling
            CheckAndApplyScaling();
        }
        break;
    }
    case WM_TRAYICON: {
        if (lParam == WM_RBUTTONUP) {
            // Show context menu
            HMENU hMenu = CreatePopupMenu();
            if (isEnabled) {
                AppendMenuW(hMenu, MF_STRING, ID_TRAY_DISABLE, L"Disable Auto-Scaling");
            } else {
                AppendMenuW(hMenu, MF_STRING, ID_TRAY_ENABLE, L"Enable Auto-Scaling");
            }

            // Add the "Start with Windows" menu item with a checkmark if enabled
            AppendMenuW(hMenu, MF_STRING | (isAutoStartEnabled ? MF_CHECKED : 0), ID_TRAY_AUTOSTART, L"Start with Windows");

            AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");

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
            MessageBoxW(hwnd, L"AutoScale Enabled", L"Status", MB_OK);
            break;
        case ID_TRAY_DISABLE:
            isEnabled = false;
            MessageBoxW(hwnd, L"AutoScale Disabled", L"Status", MB_OK);
            break;
        case ID_TRAY_AUTOSTART:
            isAutoStartEnabled = !isAutoStartEnabled;
            EnableAutoStart(isAutoStartEnabled);
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
        // Clean up the tray icon and kill the timer
        KillTimer(hwnd, IDT_TIMER1);
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    hInst = hInstance;

    // Check if auto-start is enabled
    isAutoStartEnabled = IsAutoStartEnabled();

    // Register window class
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"AutoScaleClass";
    RegisterClassW(&wc);

    // Create an invisible window
    HWND hwnd = CreateWindowW(L"AutoScaleClass", L"AutoScale", WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
                              NULL, NULL, hInst, NULL);

    // Initialize lastScaling
    lastScaling = 0;  // Or set to the current scaling if you can retrieve it

    // Main message loop
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}
