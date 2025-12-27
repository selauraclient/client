#include <pch.hpp>
#include <memory/patterns/resolver.hpp>

void EjectAndExit(HMODULE hModule) {
    fmt::println("Ejecting Selaura...");
    selaura::detail::run_cleanup();

    Sleep(100);

    fclose(stdout);
    fclose(stderr);
    fclose(stdin);
    FreeConsole();

    FreeLibraryAndExitThread(hModule, 0);
}

DWORD WINAPI SelauraProc(LPVOID lpParam) {
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

    selaura::detail::get_registry().push_back({ "sdk.dll", fakeImportResolver });
    selaura::delay_loader::load_all_imports();

    selaura::hook<&bgfx::d3d11::RenderContextD3D11::submit>::enable();

    const int target_ms = 5000;
    const int interval_ms = 100;
    int hold_time = 0;

    while (true) {
        bool keys_down = (GetAsyncKeyState(VK_RSHIFT) & 0x8000) && (GetAsyncKeyState(VK_BACK) & 0x8000);

        if (keys_down) {
            hold_time += interval_ms;
            if (hold_time % 1000 == 0) {
                fmt::println("Ejecting in {}s...", (target_ms - hold_time) / 1000);
            }
        } else {
            hold_time = 0;
        }

        if (hold_time >= target_ms) {
            break;
        }

        Sleep(interval_ms);
    }

    EjectAndExit(static_cast<HMODULE>(lpParam));

    return 0;
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (GetModuleHandleA("Minecraft.Windows.exe") == nullptr) return FALSE;
        CreateThread(nullptr, 0, &SelauraProc, hinstDLL, 0, nullptr);
    }

    return TRUE;
}