#pragma once
#include <future>
#include <vector>
#include <unordered_map>
#include <typeindex>

#include <libhat/scanner.hpp>
#include <libhat/signature.hpp>

#include "../platform/platform.hpp"
#include "sdk/core/RenderItemInHandDescription.hpp"
#include "sdk/game/uri/GameArguments.hpp"

#include "sdk/gui/ScreenView.hpp"
#include "sdk/renderer/bgfx/bgfx.hpp"
#include "sdk/world/BaseLightTextureImageBuilder.hpp"
#include "sdk/world/Dimension.hpp"
#include "sdk/client/ClientInstanceScreenModel.hpp"
#include "sdk/gui/GuiData.hpp"
#include "sdk/game/network/MinecraftPackets.hpp"

namespace selaura {
    template <auto fn>
    struct signature {
        static_assert(sizeof(fn) == 0, "signature must be specialized for this tag type");
    };

    template <auto fn>
    constexpr auto get_signature = signature<fn>::value;

    template <auto fn, typename T = std::uintptr_t>
    T resolve_signature(std::string_view module_name = {}) {
        static auto& handle = module_name.empty()
               ? get_dynamic_module()
               : get_dynamic_module(module_name);

        return reinterpret_cast<T>(
            hat::find_pattern(handle.memory_view, signature<fn>::value).get()
        );
    }

    template <auto... tags>
    void prewarm_signatures(std::string_view module_name = {}) {
        std::vector<std::future<void>> futures;

        (futures.emplace_back(std::async(std::launch::async, [module_name]() {
            (void)resolve_signature<tags, std::uintptr_t>(module_name);
        })), ...);

        for (auto& fut : futures) {
            fut.get();
        }
    }

    template <>
    struct selaura::signature<&ScreenView::setupAndRender_hk> {
        static constexpr auto value = hat::compile_signature<"48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 98 FD">();
    };

    template <>
    struct selaura::signature<&BaseLightTextureImageBuilder::createBaseLightTextureData_hk> {
        static constexpr auto value = hat::compile_signature<"48 89 5C 24 ? 48 89 54 24 ? 55 56 57 41 56 41 57 48 83 EC 40 4D 8B F1">();
    };

    template <>
    struct selaura::signature<&NetherLightTextureImageBuilder::createBaseLightTextureData_hk> {
        static constexpr auto value = hat::compile_signature<"48 89 ? ? ? 48 89 ? ? ? 55 56 57 41 ? 41 ? 48 83 EC ? 49 8B ? 49 8B ? 48 8B ? 45 33">();
    };

    template <>
    struct selaura::signature<&GameArguments::_onUri_hk> {
        static constexpr auto value = hat::compile_signature<"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4C 8B FA 48 8B F1">();
    };

    template <>
    struct selaura::signature<&bgfx::d3d11::RendererContextD3D11::submit_hk> {
        static constexpr auto value = hat::compile_signature<"48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ?">();
    };

    template <>
    struct selaura::signature<&bgfx::d3d12::RendererContextD3D12::submit_hk> {
        static constexpr auto value = hat::compile_signature<"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 0F 29 B4 24 ? ? ? ? 0F 29 BC 24 ? ? ? ? 44 0F 29 84 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4C 89 4C 24">();
    };

    template <>
    struct selaura::signature<&Dimension::getTimeOfDay_hk> {
        static constexpr auto value = hat::compile_signature<"? ? ? ? ? ? 76 05 F7 EA C1 FA 09 8B C2">();
    };

    template <>
    struct selaura::signature<&Dimension::Dimension_ctor_hk> {
        static constexpr auto value = hat::compile_signature<"48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 41 8B D9 41 8B F8">();
    };

    template <>
    struct selaura::signature<&mce::framebuilder::RenderItemInHandDescription::RenderItemInHandDescription_ctor_hk> {
        static constexpr auto value = hat::compile_signature<"48 89 ? ? ? 48 89 ? ? ? 55 56 57 41 ? 41 ? 41 ? 41 ? 48 83 EC ? 4D 8B ? 4D 8B ? 4C 8B ? 48 8B ? 45 33">();
    };

    template <>
    struct selaura::signature<&ClientInstanceScreenModel::executeCommand_hk> {
        static constexpr auto value = hat::compile_signature<"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4C 8B EA 48 8B F1 C7 44 24">();
    };

    template <>
    struct selaura::signature<&GuiData::displayClientMessage_hk> {
        static constexpr auto value = hat::compile_signature<"40 55 53 56 57 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 8B F8 48 8B DA 48 8B F1 0F 57 C0">();
    };

    template <>
    struct selaura::signature<&MinecraftPackets::createPacket_hk> {
        static constexpr auto value = hat::compile_signature<"48 89 5C 24 10 48 89 74 24 18 57 48 83 EC 50 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 48 48 8B D9 48 89">();
    };
};
