#include "pch.hpp"
#include <memory/patterns/resolver.hpp>

SafetyHookInline hk{};

void hk_submit(void* thisptr, void* a1) {
    hk.call<void>(thisptr, a1);
    fmt::println("hi");
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

    selaura::delay_loader::load_all_imports("sdk.dll", fakeImportResolver);
    auto pkt = MinecraftPackets::createPacket(MinecraftPacketIds::Text);
    fmt::println("test: {}", pkt->getName());

    hk = safetyhook::create_inline(std::bit_cast<void*>(&ScreenView::setupAndRender), &hk_submit);

    return 0;
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (GetModuleHandleA("Minecraft.Windows.exe") == nullptr) return FALSE;
        CreateThread(nullptr, 0, &SelauraProc, hinstDLL, 0, nullptr);
    }

    return TRUE;
}