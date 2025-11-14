#include "sdk.hpp"

using ResolveAddressPtr = uintptr_t(*)(uint32_t ordinal);

ResolveAddressPtr fakeImportResolver = +[](uint32_t ordinal) -> uintptr_t {
    switch (static_cast<int>(ordinal)) {
        case 1: // MinecraftPackets::createPacket
            return Selaura::Pattern<"48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 48 8B F9 48 89 4C 24 ? 33 ED 81 FA">::resolve();
        default:
            return 0x0;
    }
};

void Selaura::InitializeSDK() {
    fi::FakeImportConfig::set_config({
        .mock_dll_name = "sdk.dll",
        .resolve_address = fakeImportResolver,
    });

    fi::load_all_imports();
}