#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

#pragma comment(lib, "winmm.lib")

const wchar_t* WINDOW_CLASS = L"WEKiiClass";
const wchar_t* WINDOW_TITLE = L"WEKii";

struct FlyingThing {
    float x, y;
    float vx, vy;
    int type;
    COLORREF color;
    int life;
    int maxLife;
    std::wstring text;
};

std::vector<FlyingThing> things;
bool running = true;
int screenW = 0;
int screenH = 0;
DWORD lastSoundTime = 0;
float rainbowOffset = 0;

float mouseX = 0;
float mouseY = 0;
float mouseVX = 5.0f;
float mouseVY = 4.0f;
DWORD lastMouseMove = 0;

const wchar_t* texts[] = {
    L"SINIR BOZUCU", L"KAÇAMAZSIN", L"YAKALADIM", L"HEHEHE",
    L"BOO", L"MESAJIN VAR", L"HATA", L"SILINDI", L"VIRUS",
    L"SENI BULDUM", L"ARKANDAYIM", L"GIT BURADAN", L"BOZUK",
    L"COKTU", L"!!!", L"???", L"HAHAHA", L"KAPAT BENI",
    L"ESC BAS", L"WEKii"
};

const wchar_t* emojis[] = {
    L"😈", L"👹", L"👺", L"🤡", L"💀", L"👻", L"🐛", L"🕷️",
    L"💩", L"🤮", L"🤢", L"👽", L"🎃", L"🦟", L"🦠", L"☠️",
    L"😱", L"😵", L"🙈", L"🔥", L"💣", L"⚡", L"🌟", L"❌"
};

COLORREF randomColor() {
    return RGB(rand() % 256, rand() % 256, rand() % 256);
}

COLORREF hsvToRgb(float h, float s, float v) {
    float c = v * s;
    float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
    float m = v - c;

    float r = 0, g = 0, b = 0;

    if (h < 60)       { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else              { r = c; g = 0; b = x; }

    return RGB((int)((r + m) * 255), (int)((g + m) * 255), (int)((b + m) * 255));
}

void spawnThing() {
    FlyingThing t;
    t.x = (float)(rand() % screenW);
    t.y = (float)(rand() % screenH);
    t.vx = ((rand() % 200) / 100.0f) - 1.0f;
    t.vy = ((rand() % 200) / 100.0f) - 1.0f;
    t.type = rand() % 3;
    t.color = randomColor();
    t.maxLife = 300 + rand() % 300;
    t.life = t.maxLife;

    if (t.type == 0) {
        t.text = texts[rand() % 20];
    } else if (t.type == 1) {
        t.text = emojis[rand() % 24];
    }

    things.push_back(t);
}

void updateThings() {
    for (auto& t : things) {
        t.x += t.vx;
        t.y += t.vy;
        t.life--;

        if (t.x < 0) { t.x = 0; t.vx *= -1; }
        if (t.x > screenW - 150) { t.x = (float)(screenW - 150); t.vx *= -1; }
        if (t.y < 0) { t.y = 0; t.vy *= -1; }
        if (t.y > screenH - 60) { t.y = (float)(screenH - 60); t.vy *= -1; }
    }

    for (int i = (int)things.size() - 1; i >= 0; i--) {
        if (things[i].life <= 0) things.erase(things.begin() + i);
    }

    while (things.size() < 50) spawnThing();
}

void moveMouse() {
    DWORD now = GetTickCount();
    if (now - lastMouseMove < 30) return;
    lastMouseMove = now;

    mouseX += mouseVX;
    mouseY += mouseVY;

    if (mouseX < 0) { mouseX = 0; mouseVX = fabs(mouseVX); }
    if (mouseX > screenW - 1) { mouseX = (float)(screenW - 1); mouseVX = -fabs(mouseVX); }
    if (mouseY < 0) { mouseY = 0; mouseVY = fabs(mouseVY); }
    if (mouseY > screenH - 1) { mouseY = (float)(screenH - 1); mouseVY = -fabs(mouseVY); }

    if (rand() % 60 == 0) {
        mouseVX = ((rand() % 200) / 10.0f) - 10.0f;
        mouseVY = ((rand() % 200) / 10.0f) - 10.0f;

        if (fabs(mouseVX) < 2) mouseVX = 5;
        if (fabs(mouseVY) < 2) mouseVY = 4;
    }

    SetCursorPos((int)mouseX, (int)mouseY);
}

void drawRainbowBackground(HDC hdc) {
    int stripeWidth = 4;

    for (int x = 0; x < screenW; x += stripeWidth) {
        float hue = fmod((x * 0.3f + rainbowOffset), 360.0f);
        if (hue < 0) hue += 360.0f;

        COLORREF color = hsvToRgb(hue, 1.0f, 1.0f);

        HBRUSH brush = CreateSolidBrush(color);
        RECT stripe = {x, 0, x + stripeWidth, screenH};
        FillRect(hdc, &stripe, brush);
        DeleteObject(brush);
    }
}

void drawThings(HDC hdc) {
    SetBkMode(hdc, TRANSPARENT);

    for (auto& t : things) {
        if (t.type == 0) {
            int fontSize = 24 + (t.maxLife % 20);
            HFONT font = CreateFontW(fontSize, 0, 0, 0, FW_BLACK, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            HFONT oldFont = (HFONT)SelectObject(hdc, font);

            int textLen = (int)t.text.length();

            SetTextColor(hdc, RGB(0, 0, 0));
            for (int dx = -2; dx <= 2; dx += 2) {
                for (int dy = -2; dy <= 2; dy += 2) {
                    if (dx == 0 && dy == 0) continue;
                    TextOutW(hdc, (int)t.x + dx, (int)t.y + dy, t.text.c_str(), textLen);
                }
            }

            SetTextColor(hdc, RGB(255, 255, 255));
            TextOutW(hdc, (int)t.x, (int)t.y, t.text.c_str(), textLen);

            SelectObject(hdc, oldFont);
            DeleteObject(font);
        } else if (t.type == 1) {
            int fontSize = 40 + (t.maxLife % 30);
            HFONT font = CreateFontW(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Emoji");
            HFONT oldFont = (HFONT)SelectObject(hdc, font);

            int textLen = (int)t.text.length();

            SetTextColor(hdc, RGB(0, 0, 0));
            for (int dx = -3; dx <= 3; dx += 3) {
                for (int dy = -3; dy <= 3; dy += 3) {
                    if (dx == 0 && dy == 0) continue;
                    TextOutW(hdc, (int)t.x + dx, (int)t.y + dy, t.text.c_str(), textLen);
                }
            }

            SetTextColor(hdc, RGB(255, 255, 255));
            TextOutW(hdc, (int)t.x, (int)t.y, t.text.c_str(), textLen);

            SelectObject(hdc, oldFont);
            DeleteObject(font);
        } else {
            int r = 20 + (t.maxLife % 50);
            HBRUSH brush = CreateSolidBrush(t.color);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
            HPEN pen = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));
            HPEN oldPen = (HPEN)SelectObject(hdc, pen);

            Ellipse(hdc, (int)t.x - r, (int)t.y - r, (int)t.x + r, (int)t.y + r);

            SelectObject(hdc, oldBrush);
            SelectObject(hdc, oldPen);
            DeleteObject(brush);
            DeleteObject(pen);
        }
    }
}

void playRandomSound() {
    DWORD now = GetTickCount();
    if (now - lastSoundTime < 4000) return;
    lastSoundTime = now;

    switch (rand() % 5) {
        case 0: MessageBeep(MB_ICONHAND); break;
        case 1: MessageBeep(MB_ICONEXCLAMATION); break;
        case 2: MessageBeep(MB_ICONASTERISK); break;
        case 3: MessageBeep(MB_ICONQUESTION); break;
        case 4: Beep(200 + rand() % 800, 100); break;
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            drawRainbowBackground(hdc);
            drawThings(hdc);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    srand((unsigned)time(NULL));

    screenW = GetSystemMetrics(SM_CXSCREEN);
    screenH = GetSystemMetrics(SM_CYSCREEN);

    mouseX = (float)(screenW / 2);
    mouseY = (float)(screenH / 2);

    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = WINDOW_CLASS;

    if (!RegisterClassExW(&wc)) return 1;

    HWND hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        WINDOW_CLASS, WINDOW_TITLE,
        WS_POPUP,
        0, 0, screenW, screenH,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 1;

    for (int i = 0; i < 50; i++) spawnThing();

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    SetForegroundWindow(hwnd);

    MSG msg;
    DWORD lastUpdate = GetTickCount();

    while (running) {
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        if (!running) break;

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            running = false;
            break;
        }

        DWORD now = GetTickCount();

        moveMouse();

        if (now - lastSoundTime > 4000) {
            playRandomSound();
        }

        if (now - lastUpdate >= 33) {
            lastUpdate = now;

            rainbowOffset += 4.0f;
            if (rainbowOffset > 360.0f) rainbowOffset -= 360.0f;

            updateThings();

            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
        }

        Sleep(5);
    }

    DestroyWindow(hwnd);
    return 0;
}