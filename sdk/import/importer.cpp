//
// Created by Akashic on 7/1/2025.
//
#include <bit>
#include <print>

#include "img_delay_description.hpp"
#include "basic.hpp"
#include "iat_write.hpp"

static std::atomic_bool batching = false;

extern "C" {
    FARPROC WINAPI __delayLoadHelper2(const fi::ImgDelayDescr *pidd,
                                      FARPROC *ppfnIATEntry) {
        fi::IDD idd{pidd};
        // Means an invalid IDD was passed
        if (idd.attributes != fi::Attributes::RVA) {
            __debugbreak();
            return nullptr;
        }

        // IAT and INT offsets are the same
        const auto iat_offset = idd.offset_in_iat(ppfnIATEntry);
        const auto int_offset = iat_offset;

        auto &thunk_data = idd.import_name_table[int_offset];

        // Usually means the function wasn't marked as NONAME in the DEF
        if (!IMAGE_SNAP_BY_ORDINAL(thunk_data.u1.Ordinal)) {
            __debugbreak();
            return nullptr;
        }

        const auto resolved_address = fi::FakeImportConfig::config().resolve_address(static_cast<uint32_t>(thunk_data.u1.Ordinal));

        fi::IATWrite writer{std::bit_cast<uintptr_t>(ppfnIATEntry), [&] {*ppfnIATEntry = std::bit_cast<FARPROC>(resolved_address); }, idd};

        // Clips the top
        return reinterpret_cast<FARPROC>(resolved_address);
    }
}

namespace fi {
    void load_all_imports() {
        const auto raw_idd =
            fi::ImgDelayDescr::idd_from_base();
        const fi::IDD idd{raw_idd};

        auto iat_entry = idd.import_address_table;
        const auto iat_size = idd.iat_size();
        const auto iat_end = iat_entry + iat_size;

        for (; iat_entry < iat_end; ++iat_entry) {
            __delayLoadHelper2(raw_idd,
                               const_cast<FARPROC *>(
                                   reinterpret_cast<const FARPROC *>(iat_entry.get())));
        }
    }
}
