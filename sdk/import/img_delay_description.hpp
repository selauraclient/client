//
// Created by Akashic on 7/1/2025.
//

#ifndef IMG_DELAY_DESCRIPTION_HPP
#define IMG_DELAY_DESCRIPTION_HPP
#include "basic.hpp"
#include "rva_ptr.hpp"


namespace fi {

    using IATEntry = FARPROC*;

    enum class Attributes : DWORD { RVA = 0x1 };

    struct ImgDelayDescr {
        DWORD grAttrs;
        RVA rvaDLLName;
        RVA rvaHmod;
        RVA rvaIAT;
        RVA rvaINT;
        RVA rvaBoundIAT;
        RVA rvaUnloadIAT;
        DWORD dwTimeStamp;

        static const ImgDelayDescr *idd_from_base();
    };

    struct IDD {
        Attributes attributes;
        RVAString module_name;
        RVAPtr<HMODULE> module_handle;
        RVAPtr<const IMAGE_THUNK_DATA> import_address_table;
        RVAPtr<const IMAGE_THUNK_DATA> import_name_table;
        RVAPtr<const IMAGE_THUNK_DATA> bound_import_address_table;
        RVAPtr<const IMAGE_THUNK_DATA> unload_import_address_table;

        explicit IDD(const ImgDelayDescr *descr);

        [[nodiscard]] size_t iat_size() const;

        uintptr_t offset_in_iat(IATEntry iat_entry) const;
    };

}

#endif //IMG_DELAY_DESCRIPTION_HPP
