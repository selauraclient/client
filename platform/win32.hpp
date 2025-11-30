#pragma once
#include <pch.hpp>
#include "platform.hpp"

#include <fmt/base.h>
#include <fmt/format.h>

namespace Selaura {
    struct WindowsPlatform : Platform {
        [[nodiscard]] PlatformType GetPlatformType() const override;
        [[nodiscard]] std::filesystem::path GetSelauraFolder() const override;
        GameVersion GetGameVersion() const override;
        void InitConsole() const override;
        [[nodiscard]] std::span<std::byte> GetMinecraftModule() const override;
        void* OpenModule(const std::string& path) const override;
        void* GetFunction(void* module, const std::string& func_name) const override;

        HWND GetHWND();
        void SetTitle(const std::string& title);

        template<typename... Args>
        void SetTitle(fmt::format_string<Args...> str, Args &&... args) {
            this->SetTitle(fmt::format(str, std::forward<Args>(args)...));
        }

        void DarkModeTitlePatch();
    };
};