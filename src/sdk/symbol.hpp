#pragma once

#include <optional>
#include <string_view>
#include <stdexcept>
#include <cstdint>

#include "process.hpp"

#include <spdlog/spdlog.h>

namespace selaura {

	enum class platform {
		windows,
		android,
		linux_platform
	};

#if defined(SELAURA_WINDOWS)
	constexpr platform current_platform = platform::windows;
#elif defined(SELAURA_ANDROID)
	constexpr platform current_platform = platform::android;
#elif defined(SELAURA_LINUX)
	constexpr platform current_platform = platform::linux_platform;
#else
#error Unknown platform
#endif

	inline std::optional<uintptr_t> find_pattern(std::string_view pattern) {
		const auto parsed = hat::parse_signature(pattern);
		if (!parsed.has_value()) {
			spdlog::error("Invalid signature! {:s}", pattern);
			return std::nullopt;
		}

		static auto process = selaura::get_process_handle();

		const auto begin = process.get_process_base();
		const auto end = begin + process.get_process_size();
		const auto result = hat::find_pattern(begin, end, parsed.value());

		if (!result.has_result()) {
			return std::nullopt;
		}
		return reinterpret_cast<uintptr_t>(result.get());
	}

	template <typename TRet, typename... TArgs>
	TRet call_virtual_raw(void* thisptr, size_t index, TArgs... argList) {
		using TFunc = TRet(__fastcall*)(void*, TArgs...);
		TFunc* vtable = *reinterpret_cast<TFunc**>(thisptr);
		return vtable[index](thisptr, argList...);
	}

	namespace details {
		template<typename>
		struct resolve_func;

		template<typename R, typename... Args>
		struct resolve_func<R(Args...)> {

			template<typename T>
			struct memfn : std::type_identity<R(T::*)(Args...)> {};

			template<typename T>
			struct memfn<const T> : std::type_identity<R(T::*)(Args...) const> {};

			template<typename T>
			using memfn_t = typename memfn<T>::type;
		};

		template<typename R, typename T, typename... Args>
		struct resolve_func<R(T::*)(Args...)> {
			template<typename>
			using memfn_t = R(T::*)(Args...);
		};

		template<typename R, typename T, typename... Args>
		struct resolve_func<R(T::*)(Args...) const> {
			template<typename>
			using memfn_t = R(T::*)(Args...) const;
		};
	}

	template <typename Ret, typename Inst, typename... Args>
	Ret call_virtual(Inst* instance, size_t index, Args&&... args) {
		const auto vtable = *reinterpret_cast<const uintptr_t* const*>(instance);
		const uintptr_t addr = vtable[index];

		using Fn = Ret(Args...);
		using memfn_t = typename details::resolve_func<Fn>::template memfn_t<Inst>;

		const auto memfn = std::bit_cast<memfn_t>(addr);
		return (instance->*memfn)(std::forward<Args>(args)...);
	}

	inline uintptr_t offset_from_sig(uintptr_t sig, int offset) {
		if (sig == 0) return 0;
		return sig + offset + 4 + *reinterpret_cast<int*>(sig + offset);
	}

	template<typename Ret>
	inline Ret offset_from_sig(const uintptr_t sig, const int offset) {
		return reinterpret_cast<Ret>(offset_from_sig(sig, offset));
	}

	template <typename T>
	struct base_symbol {
		virtual void* resolve() const = 0;
		virtual ~base_symbol() = default;

		using type = T;
	};

	template <typename T>
	struct signature_symbol : base_symbol<T> {
		struct signature_info {
			std::string_view pattern;
			std::ptrdiff_t offset = 0;
		};

		std::string_view name;
		std::unordered_map<platform, signature_info> platform_signatures;
		mutable void* cached = nullptr;

		signature_symbol(std::string_view nm, std::unordered_map<platform, signature_info> list)
			: name(nm), platform_signatures(list) {
		}

		void* resolve() const override {
			auto it = platform_signatures.find(current_platform);
			if (it == platform_signatures.end()) {
				spdlog::error("No symbol for current platform: {}", name);
				return nullptr;
			}

			if (!cached) {
				auto base = find_pattern(it->second.pattern);
				if (!base.has_value()) {
					spdlog::error("Signature not valid! {}", name);
					return nullptr;
				}
				cached = reinterpret_cast<void*>(*base + it->second.offset);
			}
			return cached;
		}
	};

	struct offset_symbol {
		struct offset_info {
			uintptr_t offset;
		};

		std::string_view name;
		std::unordered_map<platform, offset_info> platform_offsets;

		offset_symbol(std::string_view nm, std::unordered_map<platform, offset_info> list)
			: name(nm), platform_offsets(list) {
		}

		uintptr_t resolve() const {
			auto it = platform_offsets.find(current_platform);
			if (it == platform_offsets.end()) {
				spdlog::error("No symbol for current platform: {}", name);
				return 0;
			}

			return it->second.offset;
		}
	};

	template <typename T>
	struct direct_symbol : base_symbol<T> {
		void* direct_address;

		direct_symbol(void* addr) : direct_address(addr) {}

		void* resolve() const override {
			return direct_address;
		}
	};
}
