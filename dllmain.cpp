// ==============================================================================
// LICENSE REVOKED - Version: 66.0 (DIRECTX SCANCODE INPUT FIX)
// ==============================================================================

#include <windows.h>
#include <string>
#include <cstring>
#include <fstream>
#include <algorithm>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>
#include <mmsystem.h> 

// --- SDK HEADERS ---
#include "scssdk_telemetry.h"
#include "eurotrucks2/scssdk_eut2.h"
#include "eurotrucks2/scssdk_telemetry_eut2.h"
#include "scssdk_telemetry_event.h"

#pragma comment(linker, "/EXPORT:scs_telemetry_init")
#pragma comment(linker, "/EXPORT:scs_telemetry_shutdown")
#pragma comment(lib, "User32.lib") 
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Winmm.lib") 

// ==============================================================================
// 1. LOCALIZATION STORAGE
// ==============================================================================
namespace Strings {
    namespace UI {
        const char* GAME_OVER_TITLE = "ADMINISTRATIVE DECISION";
        const char* FINE_TITLE = "TRAFFIC OFFENCE";
        const char* RESET_TITLE = "CLEAN DRIVING RECORD";
        const char* FINE_BODY_PRE = "Penalty points added. Current status: ";
        const char* RESET_BODY = "No violations for the required period. Points reset to 0.";
        const char* LEGAL_TEXT = "Traffic enforcement systems have detected that you have exceeded the penalty points limit.\n\n"
            "Your driving privileges are hereby suspended with immediate effect. "
            "Further operation of the vehicle is prohibited.\n\n"
            "Profile Status: LOCKED";
    }
    namespace Log {
        const char* INIT = "License Revoked v66.0 Loaded (DX Input Fix).";
        const char* CONFIG_LOAD = "Configuration loaded from .ini file.";
        const char* CONFIG_FAIL = "Config .ini not found - using defaults (EN).";
        const char* CHANNEL_OK = "Time channel registered successfully.";
        const char* CHANNEL_FAIL = "ERROR: Failed to register time channel!";
        const char* PROFILE_LOAD = "Loaded profile: ";
        const char* RESET_DONE = "Clean record! Points reset.";
        const char* FINE_DETECT = "Fine detected: ";
        const char* BAN_TRIGGER = "BAN DETECTED -> FORCE HANDBRAKE (SCANCODE) -> GAME OVER";
        const char* TIME_WARN = "Warning: Adding points with Time=0";
    }
}

scs_log_t game_log = nullptr;
void Log(const std::string& msg) { if (game_log) game_log(SCS_LOG_TYPE_message, ("[LR] " + msg).c_str()); }
void LogErr(const std::string& msg) { if (game_log) game_log(SCS_LOG_TYPE_error, ("[LR] ERROR: " + msg).c_str()); }

// ==============================================================================
// 2. CONFIGURATION & INI LOADING
// ==============================================================================
struct Config {
    static const scs_u32_t SCS_U32_NIL = (scs_u32_t)(-1);
    static const scs_u32_t CHANNEL_FLAG_NONE = 0x00000000;

    static int LIMIT;
    static int RESET_DAYS;
    static std::string LANGUAGE;

    static int PTS_COLLISION;
    static int PTS_WRONG_WAY;
    static int PTS_RED_LIGHT;
    static int PTS_SPEEDING;
    static int PTS_OTHER;

    static const int NOTIFICATION_DURATION = 3000;
    static const int GAME_OVER_DURATION = 10000;
    static constexpr const char* SOUND_FILE = "gameover.wav";
    static constexpr const char* FONT_NAME = "Roboto Condensed";

    static void SetPolish() {
        Strings::UI::GAME_OVER_TITLE = "DECYZJA ADMINISTRACYJNA";
        Strings::UI::FINE_TITLE = "WYKROCZENIE DROGOWE";
        Strings::UI::RESET_TITLE = "CZYSTE KONTO";
        Strings::UI::FINE_BODY_PRE = "Naliczono punkty karne. Stan licznika: ";
        Strings::UI::RESET_BODY = "Brak naruszen przez wymagany czas. Punkty zresetowane.";
        Strings::UI::LEGAL_TEXT = "Systemy kontroli ruchu drogowego wykryly przekroczenie limitu punktow karnych.\n\n"
            "Twoje uprawnienia do kierowania pojazdami zostaly zawieszone w trybie natychmiastowym. "
            "Dalsza jazda jest niemozliwa.\n\n"
            "Status Profilu: ZABLOKOWANY";

        Strings::Log::INIT = "Zaladowano License Revoked v66.0 (PL DX Input).";
        Strings::Log::CONFIG_LOAD = "Wczytano konfiguracje z pliku .ini";
        Strings::Log::CONFIG_FAIL = "Nie znaleziono pliku .ini - uzyto wartosci domyslnych.";
        Strings::Log::CHANNEL_OK = "Kanal czasu zarejestrowany poprawnie.";
        Strings::Log::CHANNEL_FAIL = "BLAD: Nie udalo sie zarejestrowac kanalu czasu!";
        Strings::Log::PROFILE_LOAD = "Wczytano profil: ";
        Strings::Log::RESET_DONE = "Czyste konto! Punkty zresetowane.";
        Strings::Log::FINE_DETECT = "Wykryto mandat: ";
        Strings::Log::BAN_TRIGGER = "WYKRYTO BAN -> WYMUSZONO HAMULEC (SCANCODE) -> BLOKADA";
        Strings::Log::TIME_WARN = "Ostrzezenie: Dodawanie punktow przy Czasie=0";
    }

    static void Load() {
        char path[MAX_PATH];
        HMODULE hm = NULL;
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&Load, &hm) == 0) {
            LogErr("Failed to get module handle for config path.");
            return;
        }
        GetModuleFileNameA(hm, path, sizeof(path));

        std::string configPath = path;
        size_t lastDot = configPath.find_last_of(".");
        if (lastDot != std::string::npos) configPath = configPath.substr(0, lastDot);
        configPath += ".ini";

        std::ifstream f(configPath.c_str());
        if (!f.good()) {
            Log(Strings::Log::CONFIG_FAIL);
            return;
        }
        f.close();

        char langBuf[32];
        GetPrivateProfileStringA("General", "Language", "EN", langBuf, 32, configPath.c_str());
        LANGUAGE = std::string(langBuf);

        if (LANGUAGE == "PL" || LANGUAGE == "pl") {
            SetPolish();
        }

        LIMIT = GetPrivateProfileIntA("General", "MaxPoints", 24, configPath.c_str());
        RESET_DAYS = GetPrivateProfileIntA("General", "ResetDays", 14, configPath.c_str());

        PTS_COLLISION = GetPrivateProfileIntA("Offenses", "Collision", 10, configPath.c_str());
        PTS_WRONG_WAY = GetPrivateProfileIntA("Offenses", "WrongWay", 8, configPath.c_str());
        PTS_RED_LIGHT = GetPrivateProfileIntA("Offenses", "RedLight", 6, configPath.c_str());
        PTS_SPEEDING = GetPrivateProfileIntA("Offenses", "Speeding", 2, configPath.c_str());
        PTS_OTHER = GetPrivateProfileIntA("Offenses", "Other", 1, configPath.c_str());

        Log(Strings::Log::CONFIG_LOAD);
        Log("Lang: " + LANGUAGE + ", Limit: " + std::to_string(LIMIT) + ", ResetDays: " + std::to_string(RESET_DAYS));
    }

    struct Colors {
        static const COLORREF BG = RGB(20, 20, 22);
        static const COLORREF TITLE = RGB(255, 255, 255);
        static const COLORREF TEXT = RGB(210, 210, 210);
        static const COLORREF LINE = RGB(60, 60, 60);
        static const COLORREF WARN_BG = RGB(50, 50, 50);
        static const COLORREF WARN_ACCENT = RGB(255, 170, 0);
        static const COLORREF ICON_CIRCLE = RGB(255, 170, 0);
        static const COLORREF ICON_TEXT = RGB(255, 170, 0);
    };
};

int Config::LIMIT = 24;
int Config::RESET_DAYS = 14;
std::string Config::LANGUAGE = "EN";
int Config::PTS_COLLISION = 10;
int Config::PTS_WRONG_WAY = 8;
int Config::PTS_RED_LIGHT = 6;
int Config::PTS_SPEEDING = 2;
int Config::PTS_OTHER = 1;


// ==============================================================================
// 3. LOGIC CLASS
// ==============================================================================
class LicenseRules {
public:
    int points = 0;
    unsigned int lastFineTime = 0;
    bool isBanned = false;

    enum class TimeResult { NONE, RESET_PERFORMED };

    void LoadState(int p, unsigned int t) {
        points = p;
        lastFineTime = t;
        CheckBan();
    }

    void AddPoints(int amount, unsigned int now) {
        points += amount;
        if (now > 0) lastFineTime = now;
        CheckBan();
    }

    TimeResult UpdateTime(unsigned int now) {
        if (lastFineTime == 0 && points == 0 && now > 0) {
            lastFineTime = now;
            return TimeResult::NONE;
        }

        unsigned int resetMinutes = Config::RESET_DAYS * 24 * 60;

        if (points > 0 && lastFineTime > 0 && (now > lastFineTime + resetMinutes)) {
            points = 0;
            lastFineTime = now;
            isBanned = false;
            return TimeResult::RESET_PERFORMED;
        }
        return TimeResult::NONE;
    }

private:
    void CheckBan() {
        if (points >= Config::LIMIT) isBanned = true;
    }
};

// ==============================================================================
// 4. STATE MANAGER
// ==============================================================================
class GameState {
private:
    std::mutex mtx;
    LicenseRules rules;
    scs_u32_t currentGameTime = 0;
    std::string licensePlate = "";
    bool showOverlay = false;
    bool gameOverTriggered = false;
    bool isResetNotification = false;
    DWORD notifyStartTime = 0;
    DWORD gameOverStartTime = 0;

    std::string GetSaveFileName() {
        std::lock_guard<std::mutex> lock(mtx);
        if (licensePlate.empty()) return "lr_save_unknown.txt";
        return "lr_save_" + licensePlate + ".txt";
    }

    void CheckBanTrigger() {
        if (rules.isBanned && !gameOverTriggered) {
            gameOverTriggered = true;
            gameOverStartTime = GetTickCount();
            showOverlay = true;

            Log(Strings::Log::BAN_TRIGGER);
        }
    }

public:
    static GameState& Instance() { static GameState i; return i; }

    void Load() {
        std::string filename = GetSaveFileName();
        std::ifstream file(filename);
        std::lock_guard<std::mutex> lock(mtx);
        int p = 0; unsigned int t = 0;
        if (file.is_open()) {
            file >> p >> t;
            file.close();
            Log(std::string(Strings::Log::PROFILE_LOAD) + filename + " (Pts: " + std::to_string(p) + ")");
        }
        rules.LoadState(p, t);
        CheckBanTrigger();
    }

    void Save() {
        std::string filename = GetSaveFileName();
        std::lock_guard<std::mutex> lock(mtx);
        std::ofstream file(filename);
        if (file.is_open()) {
            file << rules.points << " " << rules.lastFineTime;
            file.close();
        }
    }

    void UpdateTime(scs_u32_t time) {
        std::lock_guard<std::mutex> lock(mtx);
        currentGameTime = time;
        auto result = rules.UpdateTime(time);
        if (result == LicenseRules::TimeResult::RESET_PERFORMED) {
            showOverlay = true;
            isResetNotification = true;
            notifyStartTime = GetTickCount();
            Log(Strings::Log::RESET_DONE);
            std::ofstream file(licensePlate.empty() ? "lr_save_unknown.txt" : "lr_save_" + licensePlate + ".txt");
            if (file.is_open()) file << rules.points << " " << rules.lastFineTime;
        }
    }

    void AddPoints(int amount, const std::string& reason) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (amount <= 0) {
                Log("Fine detected but ignored (Points=0 in config): " + reason);
                return;
            }

            if (currentGameTime == 0) LogErr(Strings::Log::TIME_WARN);
            rules.AddPoints(amount, currentGameTime);
            Log(std::string(Strings::Log::FINE_DETECT) + reason + ". Pts: " + std::to_string(rules.points));

            CheckBanTrigger();

            if (!rules.isBanned && !gameOverTriggered) {
                showOverlay = true;
                isResetNotification = false;
                notifyStartTime = GetTickCount();
            }
        }
        Save();
    }

    void SetLicensePlate(const char* plate) {
        std::string newPlate = plate ? plate : "";
        for (auto& c : newPlate) if (c == ' ') c = '_';
        bool changed = false;
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (newPlate != licensePlate) { licensePlate = newPlate; changed = true; }
        }
        if (changed) Load();
    }

    struct RenderSnapshot {
        bool show, isGameOver, isReset;
        int pts;
        long long msRemaining;
    };

    RenderSnapshot GetSnapshot() {
        std::lock_guard<std::mutex> lock(mtx);
        RenderSnapshot snap;
        snap.show = showOverlay;
        snap.isGameOver = gameOverTriggered;
        snap.isReset = isResetNotification;
        snap.pts = rules.points;
        snap.msRemaining = 0;
        DWORD now = GetTickCount();
        if (snap.isGameOver) {
            snap.msRemaining = (long long)Config::GAME_OVER_DURATION - (long long)(now - gameOverStartTime);
        }
        else if (snap.show) {
            if (now - notifyStartTime > Config::NOTIFICATION_DURATION) {
                showOverlay = false;
                snap.show = false;
            }
        }
        return snap;
    }
};

// ==============================================================================
// 5. RENDERER & WINDOWING
// ==============================================================================
class Renderer {
    HFONT hTitle = NULL, hBody = NULL, hIcon = NULL;
    HFONT F(int s, int w, const char* n) { return CreateFontA(s, 0, 0, 0, w, 0, 0, 0, 1, 0, 0, 5, 0, n); }
    void RoundRectDraw(HDC hdc, int x, int y, int w, int h, int r, COLORREF c) {
        HBRUSH b = CreateSolidBrush(c); HPEN p = CreatePen(PS_NULL, 0, 0);
        SelectObject(hdc, b); SelectObject(hdc, p); RoundRect(hdc, x, y, x + w, y + h, r, r);
        DeleteObject(b); DeleteObject(p);
    }
public:
    void Draw(HDC hdc, int w, int h, const GameState::RenderSnapshot& d) {
        RECT full = { 0,0,w,h }; FillRect(hdc, &full, (HBRUSH)GetStockObject(BLACK_BRUSH));
        if (!hTitle) { hTitle = F(32, 700, Config::FONT_NAME); hBody = F(24, 400, Config::FONT_NAME); hIcon = F(36, 700, "Arial"); }
        SetBkMode(hdc, TRANSPARENT);

        if (d.isGameOver) {
            RoundRectDraw(hdc, 0, 0, w, h, 8, Config::Colors::BG);
            SelectObject(hdc, hTitle); SetTextColor(hdc, Config::Colors::TITLE);
            RECT rT = { 0,40,w,90 }; DrawTextA(hdc, Strings::UI::GAME_OVER_TITLE, -1, &rT, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            HPEN l = CreatePen(PS_SOLID, 1, Config::Colors::LINE); SelectObject(hdc, l); MoveToEx(hdc, 0, 100, 0); LineTo(hdc, w, 100); DeleteObject(l);
            SelectObject(hdc, hBody); SetTextColor(hdc, Config::Colors::TEXT);
            RECT rB = { 60,140,w - 60,h - 60 }; DrawTextA(hdc, Strings::UI::LEGAL_TEXT, -1, &rB, DT_CENTER | DT_WORDBREAK);
        }
        else {
            RoundRectDraw(hdc, 0, 0, w, h, 6, Config::Colors::WARN_BG);
            HBRUSH acc = CreateSolidBrush(Config::Colors::WARN_ACCENT);
            RECT rA = { 0,h - 4,w,h }; FillRect(hdc, &rA, acc); DeleteObject(acc);
            HPEN pen = CreatePen(PS_SOLID, 3, Config::Colors::WARN_ACCENT); SelectObject(hdc, pen); SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Ellipse(hdc, 25, (h - 44) / 2, 25 + 44, (h - 44) / 2 + 44); DeleteObject(pen);
            SelectObject(hdc, hIcon); SetTextColor(hdc, Config::Colors::WARN_ACCENT);
            RECT rI = { 25, (h - 44) / 2, 69, (h - 44) / 2 + 48 }; DrawTextA(hdc, "!", -1, &rI, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            static HFONT hS_T = F(21, 400, Config::FONT_NAME); static HFONT hS_B = F(19, 400, Config::FONT_NAME);
            int tx = 90, tw = w - 100;
            SelectObject(hdc, hS_T); SetTextColor(hdc, RGB(255, 255, 255));
            std::string t = d.isReset ? Strings::UI::RESET_TITLE : Strings::UI::FINE_TITLE;
            RECT r1 = { tx, 20, tx + tw, 50 }; DrawTextA(hdc, t.c_str(), -1, &r1, DT_LEFT | DT_TOP);
            SelectObject(hdc, hS_B); SetTextColor(hdc, RGB(220, 220, 220));
            std::string b = d.isReset ? Strings::UI::RESET_BODY : (std::string(Strings::UI::FINE_BODY_PRE) + std::to_string(d.pts) + " / " + std::to_string(Config::LIMIT));
            RECT r2 = { tx, 50, tx + tw, 90 }; DrawTextA(hdc, b.c_str(), -1, &r2, DT_LEFT | DT_TOP | DT_WORDBREAK);
        }
    }
};

HWND hOverlayWnd = NULL;
volatile bool g_running = true;
Renderer g_renderer;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_PAINT) {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps); RECT r; GetClientRect(hwnd, &r);
        HDC mdc = CreateCompatibleDC(hdc); HBITMAP bm = CreateCompatibleBitmap(hdc, r.right, r.bottom); SelectObject(mdc, bm);
        auto data = GameState::Instance().GetSnapshot();
        if (data.show) {
            g_renderer.Draw(mdc, r.right, r.bottom, data);
            if (data.isGameOver && data.msRemaining <= 200) { ShowWindow(hwnd, SW_HIDE); if (data.msRemaining <= 0) exit(0); }
        }
        else {
            RECT clr = { 0,0,r.right,r.bottom }; HBRUSH b = CreateSolidBrush(0); FillRect(mdc, &clr, b); DeleteObject(b);
        }
        BitBlt(hdc, 0, 0, r.right, r.bottom, mdc, 0, 0, SRCCOPY);
        DeleteObject(bm); DeleteDC(mdc); EndPaint(hwnd, &ps); return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

DWORD WINAPI OverlayThread(LPVOID) {
    const char* CN = "LROverlayClass";
    WNDCLASSA wc = { 0 }; wc.lpfnWndProc = WindowProc; wc.hInstance = GetModuleHandle(NULL); wc.lpszClassName = CN; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc);
    hOverlayWnd = CreateWindowExA(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, CN, "LR", WS_POPUP, 0, 0, 100, 100, NULL, NULL, wc.hInstance, NULL);
    SetLayeredWindowAttributes(hOverlayWnd, 0, 255, LWA_ALPHA);
    MSG msg;
    while (g_running) {
        HWND hGame = FindWindowA(NULL, "Euro Truck Simulator 2");
        if (!hGame) hGame = FindWindowA(NULL, "American Truck Simulator");
        if (!hGame) { Sleep(100); continue; }
        auto d = GameState::Instance().GetSnapshot();
        if (d.show) {
            if (!IsWindowVisible(hOverlayWnd)) ShowWindow(hOverlayWnd, SW_SHOWNA);
            RECT rGame; GetWindowRect(hGame, &rGame);
            int gw = rGame.right - rGame.left, gh = rGame.bottom - rGame.top;
            int w = d.isGameOver ? 900 : 450;
            int h = d.isGameOver ? 540 : 110;
            int tx = rGame.left + (gw - w) / 2;
            int ty = d.isGameOver ? rGame.top + (gh - h) / 2 : rGame.top + 100;
            SetWindowPos(hOverlayWnd, HWND_TOPMOST, tx, ty, w, h, SWP_NOACTIVATE);
            InvalidateRect(hOverlayWnd, NULL, FALSE);
            if (d.isGameOver) {
                if (GetForegroundWindow() != hOverlayWnd) { SetForegroundWindow(hOverlayWnd); SetFocus(hOverlayWnd); SetCapture(hOverlayWnd); }
                RECT cr; GetWindowRect(hOverlayWnd, &cr); ClipCursor(&cr);
            }
            else ClipCursor(NULL);
        }
        else {
            if (IsWindowVisible(hOverlayWnd)) ShowWindow(hOverlayWnd, SW_HIDE);
            ClipCursor(NULL);
        }
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageA(&msg); }
        Sleep(16);
    }
    UnregisterClassA(CN, GetModuleHandle(NULL));
    return 0;
}

// --- SDK CALLBACKS ---
SCSAPI_VOID t_time(const scs_string_t, const scs_u32_t, const scs_value_t* v, const scs_context_t) { if (v) GameState::Instance().UpdateTime(v->value_u32.value); }
SCSAPI_VOID t_conf(const scs_event_t, const void* i, const scs_context_t) {
    auto c = (const scs_telemetry_configuration_t*)i;
    if (!strcmp(c->id, "truck")) for (auto a = c->attributes; a->name; a++) if (!strcmp(a->name, "license.plate")) GameState::Instance().SetLicensePlate(a->value.value_string.value);
}
SCSAPI_VOID t_game(const scs_event_t, const void* i, const scs_context_t) {
    auto e = (const scs_telemetry_gameplay_event_t*)i;
    if (!strcmp(e->id, "player.fined")) {
        std::string off = "unknown"; for (auto a = e->attributes; a->name; a++) if (!strcmp(a->name, "fine.offence")) off = a->value.value_string.value;
        int pts = Config::PTS_OTHER;
        if (off == "crash") pts = Config::PTS_COLLISION;
        else if (off == "wrong_way") pts = Config::PTS_WRONG_WAY;
        else if (off == "red_signal") pts = Config::PTS_RED_LIGHT;
        else if (off == "speeding" || off == "speeding_camera") pts = Config::PTS_SPEEDING;
        GameState::Instance().AddPoints(pts, off);
    }
}

SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version, const scs_telemetry_init_params_t* p) {
    auto v = (const scs_telemetry_init_params_v100_t*)p; game_log = v->common.log; g_running = true;

    Config::Load();

    Log(Strings::Log::INIT);
    if (v->register_for_channel("game.time", Config::SCS_U32_NIL, SCS_VALUE_TYPE_u32, Config::CHANNEL_FLAG_NONE, t_time, nullptr) != SCS_RESULT_ok) LogErr(Strings::Log::CHANNEL_FAIL);
    else Log(Strings::Log::CHANNEL_OK);

    v->register_for_event(SCS_TELEMETRY_EVENT_configuration, t_conf, 0);
    v->register_for_event(SCS_TELEMETRY_EVENT_gameplay, t_game, 0);
    CreateThread(0, 0, OverlayThread, 0, 0, 0);
    return SCS_RESULT_ok;
}
SCSAPI_VOID scs_telemetry_shutdown(void) { g_running = false; Sleep(50); if (hOverlayWnd) ShowWindow(hOverlayWnd, SW_HIDE); GameState::Instance().Save(); }