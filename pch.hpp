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

#if defined(_MSC_VER) && defined(__clang__)
#include <platform/entt_clang.hpp>
#endif

#include <fmt/base.h>
#include <fmt/format.h>

#include <libhat/access.hpp>
#include <libhat/signature.hpp>
#include <libhat/scanner.hpp>
#include <libhat/memory_protector.hpp>
#include <libhat/process.hpp>

#include <memory/memory.hpp>
#include <memory/patterns.hpp>
#include <memory/delayloader/delayloader.hpp>

#include <sdk/network/IPacketHandlerDispatcher.hpp>
#include <sdk/network/Packet.hpp>
#include <sdk/network/MinecraftPackets.hpp>
#include <sdk/network/PacketHandlerDispatcher.hpp>

#include <sdk/renderer/ScreenView.hpp>

#include <sdk/world/Dimension.hpp>