#include "win32.hpp"

namespace Selaura {
    PlatformType WindowsPlatform::GetPlatformType() const {
        return PlatformType::Windows;
    }

    std::filesystem::path WindowsPlatform::GetSelauraFolder() const {
        PWSTR appDataPath = nullptr;
        if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath))) {
            throw std::runtime_error("Failed to get AppData path");
        }

        std::filesystem::path folder = appDataPath;
        CoTaskMemFree(appDataPath);

        folder /= R"(Minecraft Bedrock\Users\Shared\games\com.mojang\Selaura)";

        return folder;
    }

    GameVersion WindowsPlatform::GetGameVersion() const {
        GameVersion version = {};

        DWORD handle = 0;
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(nullptr, path, MAX_PATH);

        DWORD size = GetFileVersionInfoSizeW(path, &handle);
        if (size) {
            std::string buffer(size, '\0');
            if (GetFileVersionInfoW(path, handle, size, buffer.data())) {
                VS_FIXEDFILEINFO* file_info = nullptr;
                UINT len = 0;
                if (VerQueryValueW(buffer.data(), L"\\", reinterpret_cast<LPVOID*>(&file_info), &len) && file_info) {
                    version.major = HIWORD(file_info->dwFileVersionMS);
                    version.minor = LOWORD(file_info->dwFileVersionMS);
                    version.build = HIWORD(file_info->dwFileVersionLS);
                    version.revision = LOWORD(file_info->dwFileVersionLS);
                }
            }
        }

        return version;
    }

    void WindowsPlatform::InitConsole() const {
        AllocConsole();

        AttachConsole(GetCurrentProcessId());
        SetConsoleTitleA("Selaura Runtime Console");

        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        freopen_s(&fp, "CONIN$", "r", stdin);

        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD mode = 0;
            if (GetConsoleMode(hOut, &mode)) {
                SetConsoleMode(hOut, mode | 0x0004);
            }
        }
    }

    std::span<std::byte> WindowsPlatform::GetMinecraftModule() const {
        const HMODULE handle = GetModuleHandleA("Minecraft.Windows.exe");
        if (!handle) return {};

        MODULEINFO moduleInfo;
        if (!GetModuleInformation(GetCurrentProcess(), handle, &moduleInfo, sizeof(moduleInfo))) return {};
        return std::span<std::byte>{static_cast<std::byte*>(moduleInfo.lpBaseOfDll), moduleInfo.SizeOfImage};
    }

    void* WindowsPlatform::OpenModule(const std::string &path) const {
        HMODULE mod = LoadLibraryExA(path.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
        if (mod) {
            FreeLibrary(mod);
            mod = LoadLibraryA(path.c_str());
        }

        if (!mod) throw std::runtime_error("Failed to load library: " + path);
        return mod;
    }

    void* WindowsPlatform::GetFunction(void* module, const std::string& func_name) const {
        auto func = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(module), func_name.c_str()));
        if (!func) throw std::runtime_error("Failed to get function: " + func_name);
        return func;
    }

    HWND WindowsPlatform::GetHWND() {
        static HWND hwnd = nullptr;

        while (!hwnd) {
            EnumWindows([](HWND h, LPARAM lParam) -> BOOL {
                DWORD windowPid;
                GetWindowThreadProcessId(h, &windowPid);
                if (windowPid == GetCurrentProcessId() && GetParent(h) == nullptr && IsWindowVisible(h)) {
                    wchar_t className[256];
                    GetClassNameW(h, className, 256);

                    if (wcscmp(className, L"Bedrock") == 0) {
                        *reinterpret_cast<HWND*>(lParam) = h;
                        return FALSE;
                    }
                }
                return TRUE;
            }, (LPARAM)&hwnd);

            if (!hwnd)
                Sleep(100);
        }

        return hwnd;
    }

    void WindowsPlatform::SetTitle(const std::string& title) {
        SetWindowTextA(this->GetHWND(), title.c_str());
    }

    void WindowsPlatform::DarkModeTitlePatch() {
        BOOL value_true = TRUE;
        (void)DwmSetWindowAttribute(this->GetHWND(), DWMWA_USE_IMMERSIVE_DARK_MODE, &value_true, sizeof(value_true));

        RedrawWindow(this->GetHWND(), nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ERASE);
        SetWindowPos(this->GetHWND(), nullptr, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }

};