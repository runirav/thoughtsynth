#pragma once

#include <windows.h>
#include <cstdio>

// Simple standalone GUI window (not a VST editor)
// Shows EEG values in real-time
class EEGDisplayWindow
{
public:
    EEGDisplayWindow();
    ~EEGDisplayWindow();
    
    void create();
    void destroy();
    void update(float alpha, float beta, float theta, bool enabled);
    void show(bool visible);
    
private:
    HWND hwnd;
    HANDLE updateThread;
    bool running;
    
    float alphaValue;
    float betaValue;
    float thetaValue;
    bool eegEnabled;
    
    static DWORD WINAPI ThreadProc(LPVOID param);
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void paint(HDC hdc);
    void messageLoop();
};

inline EEGDisplayWindow::EEGDisplayWindow()
    : hwnd(nullptr), updateThread(nullptr), running(false),
      alphaValue(0.5f), betaValue(0.5f), thetaValue(0.5f),
      eegEnabled(true)
{
}

inline EEGDisplayWindow::~EEGDisplayWindow()
{
    destroy();
}

inline void EEGDisplayWindow::create()
{
    if (running) return;
    
    running = true;
    updateThread = CreateThread(nullptr, 0, ThreadProc, this, 0, nullptr);
}

inline void EEGDisplayWindow::destroy()
{
    running = false;
    if (updateThread)
    {
        WaitForSingleObject(updateThread, 1000);
        CloseHandle(updateThread);
        updateThread = nullptr;
    }
    if (hwnd)
    {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

inline void EEGDisplayWindow::update(float alpha, float beta, float theta, bool enabled)
{
    alphaValue = alpha;
    betaValue = beta;
    thetaValue = theta;
    eegEnabled = enabled;
    
    if (hwnd)
        InvalidateRect(hwnd, nullptr, FALSE);
}

inline void EEGDisplayWindow::show(bool visible)
{
    if (hwnd)
        ShowWindow(hwnd, visible ? SW_SHOW : SW_HIDE);
}

inline DWORD WINAPI EEGDisplayWindow::ThreadProc(LPVOID param)
{
    EEGDisplayWindow* window = (EEGDisplayWindow*)param;
    window->messageLoop();
    return 0;
}

inline void EEGDisplayWindow::messageLoop()
{
    // Register window class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "EEGDisplayWindow";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    
    RegisterClassEx(&wc);
    
    // Create window
    hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        "EEGDisplayWindow",
        "ThoughtSynth EEG Monitor",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        100, 100, 420, 340,
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );
    
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
    ShowWindow(hwnd, SW_SHOW);
    
    // Message loop
    MSG msg;
    while (running && GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

inline LRESULT CALLBACK EEGDisplayWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EEGDisplayWindow* window = (EEGDisplayWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (msg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (window)
                window->paint(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_CLOSE:
            if (window)
                window->running = false;
            return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

inline void EEGDisplayWindow::paint(HDC hdc)
{
    RECT rect;
    GetClientRect(hwnd, &rect);
    
    // Dark space background
    HBRUSH bgBrush = CreateSolidBrush(RGB(10, 10, 20));
    FillRect(hdc, &rect, bgBrush);
    DeleteObject(bgBrush);
    
    SetBkMode(hdc, TRANSPARENT);
    
    // Title
    SetTextColor(hdc, RGB(200, 200, 255));
    HFONT titleFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
    HFONT oldFont = (HFONT)SelectObject(hdc, titleFont);
    TextOut(hdc, 10, 10, "ThoughtSynth EEG Monitor", 24);
    SelectObject(hdc, oldFont);
    DeleteObject(titleFont);
    
    // Status
    if (eegEnabled)
    {
        SetTextColor(hdc, RGB(100, 255, 100));
        TextOut(hdc, 10, 40, "[ACTIVE] EEG Modulation Enabled", 32);
    }
    else
    {
        SetTextColor(hdc, RGB(255, 100, 100));
        TextOut(hdc, 10, 40, "[STATIC] EEG Modulation Disabled", 33);
    }
    
    // Band power bars
    int barY = 90;
    int barHeight = 40;
    int barSpacing = 70;
    
    const char* labels[] = {"Alpha (8-13Hz) - Filter", "Beta (13-30Hz) - LFO Rate", "Theta (4-8Hz) - LFO Depth"};
    float values[] = {alphaValue, betaValue, thetaValue};
    COLORREF colors[] = {RGB(100, 200, 255), RGB(255, 150, 100), RGB(150, 255, 150)};
    
    for (int i = 0; i < 3; i++)
    {
        int y = barY + i * barSpacing;
        
        // Label
        SetTextColor(hdc, RGB(180, 180, 200));
        TextOut(hdc, 10, y, labels[i], strlen(labels[i]));
        
        // Bar background
        RECT barRect = {10, y + 20, 390, y + 20 + barHeight};
        HBRUSH barBg = CreateSolidBrush(RGB(30, 30, 40));
        FillRect(hdc, &barRect, barBg);
        DeleteObject(barBg);
        
        // Bar fill
        int fillWidth = (int)(380 * values[i]);
        if (fillWidth > 0)
        {
            RECT fillRect = {10, y + 20, 10 + fillWidth, y + 20 + barHeight};
            HBRUSH fillBrush = CreateSolidBrush(colors[i]);
            FillRect(hdc, &fillRect, fillBrush);
            DeleteObject(fillBrush);
        }
        
        // Value text
        char valueText[32];
        sprintf(valueText, "%.0f%%", values[i] * 100.0f);
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, OPAQUE);
        SetBkColor(hdc, RGB(30, 30, 40));
        TextOut(hdc, 180, y + 32, valueText, strlen(valueText));
        SetBkMode(hdc, TRANSPARENT);
    }
}
