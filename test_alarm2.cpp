#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>

// Globals
HWND g_hwndBg = nullptr;
HWND g_hwndAlarm = nullptr;

// Forward declarations
LRESULT CALLBACK BgWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AlarmWndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    // Register background window class
    WNDCLASS wcBg = {};
    wcBg.lpfnWndProc = BgWndProc;
    wcBg.hInstance = hInstance;
    wcBg.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcBg.hbrBackground = CreateSolidBrush(RGB(200, 0, 0));
    wcBg.lpszClassName = L"BgClass";
    RegisterClass(&wcBg);

    // Register alarm window class
    WNDCLASS wcAlarm = {};
    wcAlarm.lpfnWndProc = AlarmWndProc;
    wcAlarm.hInstance = hInstance;
    wcAlarm.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcAlarm.hbrBackground = CreateSolidBrush(RGB(255, 0, 0));
    wcAlarm.lpszClassName = L"AlarmClass";
    RegisterClass(&wcAlarm);

    // Get primary monitor dimensions
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    // Create fullscreen translucent background (WS_EX_LAYERED for opacity)
    g_hwndBg = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED,
        L"BgClass", L"",
        WS_POPUP,
        0, 0, screenW, screenH,
        nullptr, nullptr, hInstance, nullptr);

    // Set background opacity to 60% (0.6 * 255 = 153)
    SetLayeredWindowAttributes(g_hwndBg, 0, 153, LWA_ALPHA);

    // Alarm window size
    int alarmW = 1000;
    int alarmH = 400;
    int alarmX = (screenW - alarmW) / 2;
    int alarmY = (screenH - alarmH) / 2;

    // Create opaque alarm window (owned by background so it stays above)
    g_hwndAlarm = CreateWindowEx(
        WS_EX_TOPMOST,
        L"AlarmClass", L"",
        WS_POPUP,
        alarmX, alarmY, alarmW, alarmH,
        g_hwndBg, nullptr, hInstance, nullptr);

    ShowWindow(g_hwndBg, SW_SHOW);
    UpdateWindow(g_hwndBg);
    ShowWindow(g_hwndAlarm, SW_SHOW);
    UpdateWindow(g_hwndAlarm);

    // Give alarm window focus so it receives keyboard input
    SetForegroundWindow(g_hwndAlarm);
    SetFocus(g_hwndAlarm);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK BgWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK AlarmWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rc;
        GetClientRect(hWnd, &rc);

        // Fill background red (in case of
        // repaint
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
        FillRect(hdc, &rc, hBrush);
        DeleteObject(hBrush);

        // Create big bold font
        HFONT hFont = CreateFont(
            -96,           // height (~72pt at 96dpi: 72*96/72=96 pixels, negative for char height)
            0, 0, 0,
            FW_BOLD,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_SWISS,
            L"Arial");

        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);

        DrawText(hdc, L"Feuerwehreinsatz!", -1, &rc,
                 DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            DestroyWindow(g_hwndAlarm);
            DestroyWindow(g_hwndBg);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}