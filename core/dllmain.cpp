#include <pch.hpp>
#include <memory/patterns/resolver.hpp>

#include <memory/hooks/CoreHooks.cpp>
#include <memory/hooks/D3DHooks.cpp>
#include <memory/hooks/InputHooks.cpp>

void EjectAndExit(HMODULE hModule) {
    spdlog::info("Ejecting Selaura...");
    selaura::remove_wndproc(FindWindowA("Bedrock", nullptr));
    selaura::detail::run_cleanup();

    Sleep(100);

    spdlog::shutdown();
    selaura::get<selaura::console>().shutdown();

    FreeLibraryAndExitThread(hModule, 0);
}

void drawMenu(selaura::render_event& ev) {
    using namespace selaura;
    auto menu_width = ev.screenWidth * 0.55;
    auto menu_height = ev.screenHeight * 0.65;

    auto menu_x = (ev.screenWidth - menu_width) / 2;
    auto menu_y = (ev.screenHeight - menu_height) / 2;

    auto sidebar_width = menu_width * 0.27;

    sui::panel(menu_x, menu_y, menu_width, menu_height, 15);
    sui::panel(menu_x, menu_y, sidebar_width, menu_height, 15, sui::panel_types::raised);
    sui::logo(menu_x + 15, menu_y + 15, 32);
    sui::text(menu_x + 55, menu_y + 38, 24, "selaura");
}

void drawFps(selaura::render_event& ev) {
    auto& renderer = selaura::get<selaura::renderer>();
    std::string text = fmt::format("{:.0f} FPS", ev.fps);

    auto fps_text = renderer.get_text_size(text, 24);
    auto fps_padding = glm::vec2{35, 20};
    auto fps_size = fps_text + fps_padding;

    float rect_x = 20.0f;
    float rect_y = 20.0f;

    float text_x = rect_x + (fps_size.x / 2.0f) - (fps_text.x / 2.0f);
    float text_y = rect_y + (fps_size.y * 0.5f) + (fps_text.y * 0.5f);

    renderer.draw_filled_rect(rect_x, rect_y, fps_size.x, fps_size.y, 5.0f, {24, 24, 24, 255});
    renderer.draw_text(text, text_x, text_y, 24, {255, 255, 255, 255});
}

DWORD WINAPI SelauraProc(LPVOID lpParam) {
    spdlog::stopwatch inject_timer;
    selaura::service_manager = std::make_unique<selaura::service_manager_impl>();
    auto sink = std::make_shared<selaura::console_sink>();

    auto logger = std::make_shared<spdlog::logger>("Selaura", sink);
    logger->set_pattern("[%H:%M:%S] [%l] %v");

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace);

    spdlog::stopwatch import_timer;
    selaura::detail::get_registry().push_back({ "sdk.dll", fakeImportResolver });
    selaura::delay_loader::load_all_imports();
    spdlog::debug("MCAPI imports completed in {:.0f}ms", import_timer.elapsed().count() * 1000);

    spdlog::stopwatch hook_timer;
    selaura::hook<&MinecraftGame::_update>::enable();
    selaura::hook<&bgfx::d3d11::RenderContextD3D11::submit>::enable();
    selaura::init_wndproc(FindWindowA("Bedrock", nullptr));
    spdlog::debug("Hooks created in {:.0f}ms", hook_timer.elapsed().count() * 1000);

    spdlog::debug("Injection completed in {:.0f}ms", inject_timer.elapsed().count() * 1000);

    //selaura::get<selaura::event_manager>().subscribe(&drawMenu);
    selaura::get<selaura::event_manager>().subscribe(&drawFps);

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