#include <pch.hpp>
#include <memory/patterns/resolver.hpp>

void EjectAndExit(HMODULE hModule) {
    spdlog::info("Ejecting Selaura...");
    selaura::detail::run_cleanup();

    Sleep(100);

    spdlog::shutdown();
    selaura::console->shutdown();

    FreeLibraryAndExitThread(hModule, 0);
}

DWORD WINAPI SelauraProc(LPVOID lpParam) {
    selaura::console = std::make_unique<selaura::console_impl>();
    auto sink = std::make_shared<selaura::console_sink>();
    spdlog::set_default_logger(std::make_shared<spdlog::logger>("Selaura", sink));

    selaura::detail::get_registry().push_back({ "sdk.dll", fakeImportResolver });
    selaura::delay_loader::load_all_imports();

    selaura::hook<&bgfx::d3d11::RenderContextD3D11::submit>::enable();

    // todo: finish event system

    const int target_ms = 5000;
    const int interval_ms = 100;
    int hold_time = 0;

    while (true) {
        bool keys_down = (GetAsyncKeyState(VK_RSHIFT) & 0x8000) && (GetAsyncKeyState(VK_BACK) & 0x8000);

        if (keys_down) {
            hold_time += interval_ms;
            if (hold_time % 1000 == 0) {
                spdlog::info("Ejecting in {}s...", (target_ms - hold_time) / 1000);
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