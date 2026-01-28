#include "third_person_nametag.hpp"
#include <memory/patterns.hpp>
#include <sdk/bedrock/renderer/game/LevelRendererPlayer.hpp>

namespace selaura {
    void third_person_nametag::on_enable() {
        ptr = std::bit_cast<void*>(&LevelRendererPlayer::$extractNameTags_localPlayerCheck);
        if (!ptr) return;

        std::memcpy(original_bytes.data(), ptr, size);
        hat::memory_protector prot(
            reinterpret_cast<uintptr_t>(ptr),
            size,
            hat::protection::Read |
            hat::protection::Write |
            hat::protection::Execute
        );

        if (prot.is_set()) {
            std::memset(ptr, 0x90, size);
        }
    }

    void third_person_nametag::on_disable() {
        if (!ptr) return;

        hat::memory_protector prot(
            reinterpret_cast<uintptr_t>(ptr),
            size,
            hat::protection::Read |
            hat::protection::Write |
            hat::protection::Execute
        );

        if (prot.is_set()) {
            std::memcpy(ptr, original_bytes.data(), size);
        }
    }
};