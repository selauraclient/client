#include "sdk.hpp"

int add_impl(int a, int b) {return a + b;}
int sub_impl(int a, int b) {return a - b;}

using ResolveAddressPtr = uintptr_t(*)(uint32_t ordinal);

ResolveAddressPtr fakeImportResolver = +[](uint32_t ordinal) -> uintptr_t {
    switch (static_cast<int>(ordinal)) {
        case 1:
            return reinterpret_cast<uintptr_t>(add_impl);
        case 2:
            return reinterpret_cast<uintptr_t>(sub_impl);
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