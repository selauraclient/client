//
// Created by Akashic on 7/1/2025.
//
#include "basic.hpp"

#include <bit>

namespace fi {
    IMAGE_NT_HEADERS *image_header_from_base()  {
        auto dos = &__ImageBase;

        if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
            return nullptr;
        }

        auto nt = std::bit_cast<IMAGE_NT_HEADERS *>(
            std::bit_cast<std::byte *>(dos) + dos->e_lfanew);

        if (nt->Signature != IMAGE_NT_SIGNATURE) {
            return nullptr;
        }

        return nt;
    }

    static FakeImportConfig cfg;
    const FakeImportConfig & FakeImportConfig::config() {
        return cfg;
    }

    void FakeImportConfig::set_config(FakeImportConfig &&config) {
        cfg = std::move(config);
    }
}
