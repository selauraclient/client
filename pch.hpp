#pragma once
#define CLIENT_VERSION "0.10"
#define MCAPI __declspec(dllimport)

#define NOMINMAX
#include <dwmapi.h>
#include <Psapi.h>
#include <shlobj_core.h>
#include <Windows.h>
#include <winrt/base.h>

#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#include <exception>
#include <filesystem>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <sdk/import/basic.hpp>
#include <sdk/sdk.hpp>
#include <platform/win32.hpp>

#include <safetyhook.hpp>

#include <entt/entt.hpp>
#include <entt/core/type_info.hpp>

#if defined(_MSC_VER) && defined(__clang__)
#include <platform/entt_clang.hpp>
#endif

#include <fmt/base.h>
#include <fmt/format.h>

#include <libhat/access.hpp>
#include <libhat/signature.hpp>
#include <libhat/scanner.hpp>

#include <memory/patterns.hpp>

#include <sdk/mc/network/IPacketHandlerDispatcher.hpp>
#include <sdk/mc/network/Packet.hpp>
#include <sdk/mc/network/MinecraftPackets.hpp>
#include <sdk/mc/network/PacketHandlerDispatcher.hpp>