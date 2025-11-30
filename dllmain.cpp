#include "pch.hpp"

DWORD WINAPI SelauraProc(LPVOID lpParam) {
    auto platform = std::make_unique<Selaura::WindowsPlatform>();
    auto version = platform->GetGameVersion();

    platform->SetTitle(
        "Selaura Client ({}.{}.{}/{}-master)",
        version.major,
        version.minor,
        version.build,
        CLIENT_VERSION);

    platform->DarkModeTitlePatch();
    platform->InitConsole();

    Selaura::InitializeSDK();
    auto pkt = MinecraftPackets::createPacket(MinecraftPacketIds::Text);
    fmt::println("test: {}", pkt->getName());

    return 0;
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (GetModuleHandleA("Minecraft.Windows.exe") == nullptr) return FALSE;
        CreateThread(nullptr, 0, &SelauraProc, hinstDLL, 0, nullptr);
    }

    return TRUE;
}