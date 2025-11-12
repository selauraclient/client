//
// Created by Akashic on 7/1/2025.
//
#include "img_delay_description.hpp"

#include <bit>
#include <stdexcept>


namespace fi {
    const ImgDelayDescr * ImgDelayDescr::idd_from_base() {
        auto* image_header = image_header_from_base();
        if (!image_header) {
            return nullptr;
        }

        const auto& entry = image_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT];
        if (entry.Size == 0) {
            return nullptr;
        }

        auto current_idd = RVAPtr<const ImgDelayDescr>(
            entry.VirtualAddress);

        RVAString name{current_idd->rvaDLLName};
        while (current_idd->rvaDLLName != 0) {

            if (FakeImportConfig::config().mock_dll_name == name.get()) {
                return current_idd;
            }

            ++current_idd;
            (name = current_idd->rvaDLLName);
        }

        return nullptr;
    }

    IDD::IDD(const ImgDelayDescr *descr) {
        this->attributes = static_cast<Attributes>(descr->grAttrs);
        this->module_name = descr->rvaDLLName;
        this->module_handle = descr->rvaHmod;
        this->import_address_table = descr->rvaIAT;
        this->import_name_table = descr->rvaINT;
        this->bound_import_address_table = descr->rvaBoundIAT;
        this->unload_import_address_table = descr->rvaUnloadIAT;
    }

    size_t IDD::iat_size() const {
        size_t ret = 0;
        const auto *itd = this->import_address_table.get();
        while (itd->u1.Function) {
            ret++;
            itd++;
        }
        return ret;
    }

    uintptr_t IDD::offset_in_iat(const IATEntry iat_entry) const {
        const auto* iat_entry_true = std::bit_cast<const IMAGE_THUNK_DATA*>(iat_entry);
        return std::bit_cast<uintptr_t>(iat_entry_true - this->import_address_table);
    }

}
