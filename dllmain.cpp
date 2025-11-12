//
// Created by chyves on 11/11/2025.
//

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

    fmt::println("addition: {}", add(1, 2));

    using test = Selaura::Pattern<"80 B9 ? ? ? ? ? 74 ? C6 81 ? ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 90", Selaura::SignatureOffset<Selaura::Ref, 2>>;
    fmt::println("test: {}", test::resolve());

    return 0;
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (GetModuleHandleA("Minecraft.Windows.exe") == nullptr) return FALSE;
        CreateThread(nullptr, 0, &SelauraProc, hinstDLL, 0, nullptr);
    }

    return TRUE;
}