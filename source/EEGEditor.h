#pragma once

#include <windows.h>
#include <cmath>

// Simple VST GUI Editor
class EEGEditor : public AEffEditor
{
public:
    EEGEditor(AudioEffect* effect);
    virtual ~EEGEditor();
    
    virtual bool getRect(ERect** rect);
    virtual bool open(void* ptr);
    virtual void close();
    virtual void idle();
    
    // Update EEG values from synth
    void setEEGValues(float alpha, float beta, float theta);
    void setEnabled(bool enabled);
    
private:
    HWND hwnd;
    ERect rectangle;
    
    // EEG display data
    float alphaValue;
    float betaValue;
    float thetaValue;
    bool eegEnabled;
    
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void paint(HDC hdc);
};

EEGEditor::EEGEditor(AudioEffect* effect)
    : AEffEditor(effect), hwnd(nullptr),
      alphaValue(0.5f), betaValue(0.5f), thetaValue(0.5f),
      eegEnabled(true)
{
    rectangle.top = 0;
    rectangle.left = 0;
    rectangle.bottom = 300;  // Height
    rectangle.right = 400;   // Width
}

EEGEditor::~EEGEditor()
{
    close();
}

bool EEGEditor::getRect(ERect** rect)
{
    *rect = &rectangle;
    return true;
}

bool EEGEditor::open(void* ptr)
{
    // Create window
    HWND parent = (HWND)ptr;
    
    hwnd = CreateWindowEx(
        0,
        "STATIC",
        "",
        WS_CHILD | WS_VISIBLE,
        0, 0, 400, 300,
        parent,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );
    
    if (!hwnd)
        return false;
    
    // Store this pointer for WindowProc
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
    
    // Subclass the window
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);
    
    return true;
}

void EEGEditor::close()
{
    if (hwnd)
    {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

void EEGEditor::idle()
{
    // Redraw the window
    if (hwnd)
        InvalidateRect(hwnd, nullptr, FALSE);
}

void EEGEditor::setEEGValues(float alpha, float beta, float theta)
{
    alphaValue = alpha;
    betaValue = beta;
    thetaValue = theta;
}

void EEGEditor::setEnabled(bool enabled)
{
    eegEnabled = enabled;
}

LRESULT CALLBACK EEGEditor::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EEGEditor* editor = (EEGEditor*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (msg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (editor)
                editor->paint(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_LBUTTONDOWN:
        {
            // Toggle EEG on click
            if (editor)
            {
                editor->eegEnabled = !editor->eegEnabled;
                // Tell plugin to update parameter
                editor->effect->setParameter(0, editor->eegEnabled ? 1.0f : 0.0f);
            }
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void EEGEditor::paint(HDC hdc)
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
    TextOut(hdc, 10, 10, "ThoughtSynth EEG", 17);
    
    // Status
    if (eegEnabled)
    {
        SetTextColor(hdc, RGB(100, 255, 100));
        TextOut(hdc, 10, 30, "● EEG ACTIVE (click to disable)", 33);
    }
    else
    {
        SetTextColor(hdc, RGB(255, 100, 100));
        TextOut(hdc, 10, 30, "○ EEG OFF (click to enable)", 29);
    }
    
    // Band power bars
    int barY = 80;
    int barHeight = 40;
    int barSpacing = 60;
    
    const char* labels[] = {"Alpha (8-13Hz)", "Beta (13-30Hz)", "Theta (4-8Hz)"};
    float values[] = {alphaValue, betaValue, thetaValue};
    COLORREF colors[] = {RGB(100, 200, 255), RGB(255, 150, 100), RGB(150, 255, 150)};
    
    for (int i = 0; i < 3; i++)
    {
        int y = barY + i * barSpacing;
        
        // Label
        SetTextColor(hdc, RGB(180, 180, 200));
        TextOut(hdc, 10, y, labels[i], strlen(labels[i]));
        
        // Bar background
        RECT barRect = {150, y + 5, 380, y + 5 + barHeight};
        HBRUSH barBg = CreateSolidBrush(RGB(30, 30, 40));
        FillRect(hdc, &barRect, barBg);
        DeleteObject(barBg);
        
        // Bar fill
        int fillWidth = (int)(230 * values[i]);
        if (fillWidth > 0)
        {
            RECT fillRect = {150, y + 5, 150 + fillWidth, y + 5 + barHeight};
            HBRUSH fillBrush = CreateSolidBrush(colors[i]);
            FillRect(hdc, &fillRect, fillBrush);
            DeleteObject(fillBrush);
        }
        
        // Value text
        char valueText[32];
        sprintf(valueText, "%.0f%%", values[i] * 100.0f);
        SetTextColor(hdc, RGB(255, 255, 255));
        TextOut(hdc, 385, y + 15, valueText, strlen(valueText));
    }
}
