#pragma once

#define CLIENT_VERSION "0.10"
#define MCAPI __declspec(dllimport)
#define SELAURA_EXPORT extern "C" __declspec(dllexport)

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <dwmapi.h>
#include <Psapi.h>
#include <shlobj_core.h>
#include <GameInput.h>
#include <winrt/base.h>

#include <d3d11.h>
#include <d3d12.h>
#include <d3d11on12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

// STL
#include <atomic>
#include <bit>
#include <exception>
#include <filesystem>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <span>
#include <string>
#include <tuple>
#include <typeindex>
#include <type_traits>
#include <vector>

#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <fmt/base.h>
#include <fmt/format.h>
#include <libhat/access.hpp>
#include <libhat/signature.hpp>
#include <libhat/scanner.hpp>
#include <libhat/memory_protector.hpp>
#include <libhat/process.hpp>
#include <safetyhook.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <spdlog/sinks/sink.h>

#include <core/resource.hpp>