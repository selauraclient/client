#pragma once
#include <pch.hpp>

namespace selaura {
    namespace detail {
        struct img_delay_descriptor {
            DWORD grAttrs;
            DWORD rvaDLLName;
            DWORD rvaHmod;
            DWORD rvaIAT;
            DWORD rvaINT;
            DWORD rvaBoundIAT;
            DWORD rvaUnloadIAT;
            DWORD dwTimeStamp;
        };

        extern "C" IMAGE_DOS_HEADER __ImageBase;
        using resolver_fn = std::uintptr_t(*)(std::uint64_t);

        struct loader_config {
            std::string_view target_dll;
            resolver_fn resolver;
        };

        inline std::vector<loader_config>& get_registry() {
            static std::vector<loader_config> registry;
            return registry;
        }
    };

    class delay_loader {
    public:
        static void load_all_imports(std::string_view target_dll, detail::resolver_fn resolver) {
            const auto* pidd = find_descriptor(target_dll);
            if (!pidd) return;

            const auto iat = get_rva_ptr<IMAGE_THUNK_DATA>(pidd->rvaIAT);
            const auto int_table = get_rva_ptr<IMAGE_THUNK_DATA>(pidd->rvaINT);

            size_t count = 0;
            while (int_table[count].u1.Function) count++;

            for (size_t i = 0; i < count; ++i) {
                if (IMAGE_SNAP_BY_ORDINAL(int_table[i].u1.Ordinal)) {
                    uint32_t ord = static_cast<uint32_t>(IMAGE_ORDINAL(int_table[i].u1.Ordinal));
                    iat[i].u1.Function = resolver(ord);
                }
            }
        }

        static FARPROC WINAPI global_handler(const detail::img_delay_descriptor* pidd, FARPROC* ppfnIATEntry) {
            const char* dll_name = get_rva_ptr<const char>(pidd->rvaDLLName);

            detail::resolver_fn active_resolver = nullptr;
            for (const auto& entry : detail::get_registry()) {
                if (entry.target_dll == dll_name) {
                    active_resolver = entry.resolver;
                    break;
                }
            }

            if (!active_resolver) return nullptr;

            const auto iat_base = get_rva_ptr<IMAGE_THUNK_DATA>(pidd->rvaIAT);
            const auto int_base = get_rva_ptr<IMAGE_THUNK_DATA>(pidd->rvaINT);
            const auto index = reinterpret_cast<IMAGE_THUNK_DATA*>(ppfnIATEntry) - iat_base;

            const auto& thunk = int_base[index];
            std::uintptr_t resolved_addr = 0x0;

            if (IMAGE_SNAP_BY_ORDINAL(thunk.u1.Ordinal)) {
                const uint32_t ordinal = static_cast<uint32_t>(IMAGE_ORDINAL(thunk.u1.Ordinal));
                resolved_addr = active_resolver(ordinal);
            }

            if (resolved_addr) {
                write_iat(ppfnIATEntry, resolved_addr);
            }

            return reinterpret_cast<FARPROC>(resolved_addr);
        }

    private:
        template <typename T>
        static T* get_rva_ptr(uint32_t rva) {
            return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(&detail::__ImageBase) + rva);
        }

        static void write_iat(FARPROC* entry, uintptr_t value) {
            DWORD old_prot;
            if (VirtualProtect(entry, sizeof(uintptr_t), PAGE_READWRITE, &old_prot)) {
                *entry = reinterpret_cast<FARPROC>(value);
                VirtualProtect(entry, sizeof(uintptr_t), old_prot, &old_prot);
            }
        }

        static const detail::img_delay_descriptor* find_descriptor(std::string_view target_dll) {
            auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(&detail::__ImageBase);
            auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<uint8_t*>(dos) + dos->e_lfanew);

            const auto& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT];
            if (dir.Size == 0) return nullptr;

            auto* current = get_rva_ptr<detail::img_delay_descriptor>(dir.VirtualAddress);
            while (current->rvaDLLName) {
                if (target_dll == get_rva_ptr<const char>(current->rvaDLLName)) {
                    return current;
                }
                current++;
            }
            return nullptr;
        }
    };
}

extern "C" FARPROC WINAPI __delayLoadHelper2(const selaura::detail::img_delay_descriptor* pidd, FARPROC* ppfnIATEntry) {
    return selaura::delay_loader::global_handler(pidd, ppfnIATEntry);
}