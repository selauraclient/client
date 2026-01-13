#include <pch.hpp>

#include <core/service_manager.hpp>
//#include <core/renderer/sui.hpp>
#include <core/service/console_sink.hpp>
#include <memory/cleanup.hpp>
#include <memory/hook.hpp>
#include <memory/delayloader/delayloader.hpp>
#include <memory/hooks/CoreHooks.cpp>
#include <memory/hooks/D3DHooks.cpp>
#include <memory/hooks/InputHooks.cpp>
#include <memory/patterns/resolver.hpp>
#include <sdk/core/GameInput_GDK.h>
#include <sdk/core/MinecraftGame.hpp>
#include <sdk/renderer/bgfx.hpp>

void EjectAndExit(HMODULE hModule) {
    spdlog::info("Ejecting Selaura...");
    selaura::remove_wndproc(FindWindowA("Bedrock", nullptr));
    selaura::detail::run_cleanup();

    Sleep(100);

    spdlog::shutdown();
    sgfx::shutdown();
    selaura::get<selaura::console>().shutdown();

    FreeLibraryAndExitThread(hModule, 0);
}

DWORD WINAPI SelauraProc(LPVOID lpParam) {
    spdlog::stopwatch inject_timer;
    selaura::service_manager = std::make_unique<selaura::service_manager_impl>();
    auto sink = std::make_shared<selaura::console_sink>();
    auto logger = std::make_shared<spdlog::logger>(
        "Selaura",
        sink
    );

    logger->set_pattern("[%H:%M:%S] [%l] %v");

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace);

    spdlog::stopwatch import_timer;
    selaura::detail::get_registry().push_back({ "sdk.dll", fakeImportResolver });
    selaura::delay_loader::load_all_imports();
    spdlog::debug("MCAPI imports completed in {:.0f}ms", import_timer.elapsed().count() * 1000);

    spdlog::stopwatch hook_timer;
    selaura::hook<&MinecraftGame::_update>::enable();
    selaura::hook<&bgfx::d3d11::RendererContextD3D11::submit>::enable();
    selaura::hook<&bgfx::d3d12::RendererContextD3D12::submit>::enable();
    selaura::init_wndproc(FindWindowA("Bedrock", nullptr));
    spdlog::debug("Hooks created in {:.0f}ms", hook_timer.elapsed().count() * 1000);
    spdlog::debug("Injection completed in {:.0f}ms", inject_timer.elapsed().count() * 1000);

    //selaura::get<selaura::event_manager>().subscribe(&drawMenu);
    //selaura::get<selaura::event_manager>().subscribe(&drawFps);

    GameInput::v2::IGameInput* game_input = nullptr;
    if (FAILED(GameInput::v2::GameInputCreate(&game_input))) spdlog::info("No GameInput");
    selaura::hook<&GameInput::v2::IGameInput::GetCurrentReading>::enable(game_input, 4);

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