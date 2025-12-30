#pragma once
#define CLIENT_VERSION "0.10"
#define MCAPI __declspec(dllimport)
#define SELAURA_EXPORT extern "C" __declspec(dllexport)

#define NOMINMAX
#include <dwmapi.h>
#include <Psapi.h>
#include <shlobj_core.h>
#include <Windows.h>
#include <GameInput.h>
#include <winrt/base.h>

#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#include <bit>
#include <exception>
#include <filesystem>
#include <mutex>
#include <shared_mutex>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <safetyhook.hpp>

#include <entt/entt.hpp>
#include <entt/core/type_info.hpp>

#include <fmt/base.h>
#include <fmt/format.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink-inl.h>
#include <spdlog/sinks/stdout_color_sinks-inl.h>
#include <spdlog/stopwatch.h>

#include <libhat/access.hpp>
#include <libhat/signature.hpp>
#include <libhat/scanner.hpp>
#include <libhat/memory_protector.hpp>
#include <libhat/process.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>

#include <memory/cleanup.hpp>
#include <memory/memory.hpp>
#include <memory/patterns.hpp>
#include <memory/delayloader/delayloader.hpp>

#include <memory/detour.hpp>
#include <memory/hook.hpp>

#include <sdk/core/SubClientId.hpp>
#include <sdk/core/ClientInstance.hpp>
#include <sdk/core/MinecraftGame.hpp>
#include <sdk/core/wndproc.hpp>

#include <sdk/network/IPacketHandlerDispatcher.hpp>
#include <sdk/network/Packet.hpp>
#include <sdk/network/MinecraftPackets.hpp>
#include <sdk/network/PacketHandlerDispatcher.hpp>

#include <sdk/renderer/bgfx.hpp>
#include <sdk/renderer/LevelRenderer.hpp>
#include <sdk/renderer/ScreenView.hpp>

#include <sdk/world/Dimension.hpp>

#include <memory/hooks/CoreHooks.hpp>
#include <memory/hooks/D3DHooks.hpp>
#include <memory/hooks/InputHooks.hpp>

#include <core/console.hpp>
#include <core/console_sink.hpp>