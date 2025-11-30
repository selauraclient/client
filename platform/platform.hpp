#pragma once
#include <pch.hpp>

namespace Selaura {
    enum class PlatformType : int {
        Windows = 0
    };

    struct GameVersion {
        int major;
        int minor;
        int build;
        int revision;
    };

    struct Platform {
        virtual ~Platform() = default;
        [[nodiscard]] virtual PlatformType GetPlatformType() const = 0;

        [[nodiscard]] virtual std::filesystem::path GetSelauraFolder() const = 0;
        virtual GameVersion GetGameVersion() const = 0;

        virtual void InitConsole() const = 0;

        [[nodiscard]] virtual std::span<std::byte> GetMinecraftModule() const = 0;
        virtual void* OpenModule(const std::string& path) const = 0;
        virtual void* GetFunction(void* module, const std::string& func_name) const = 0;
    };
};