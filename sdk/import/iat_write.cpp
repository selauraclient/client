//
// Created by Akashic on 7/1/2025.
//

#include "basic.hpp"
#include "iat_write.hpp"

#include <bit>
#include <libhat/memory_protector.hpp>
#include <libhat/process.hpp>

namespace fi {

    std::mutex IATWrite::IatMutex = std::mutex{};

    IATWrite::IATWrite(uintptr_t iat_address, const std::function<void()>& callback, const IDD& idd) {
        IATWrite::IatMutex.lock();

        const auto memory_size = idd.iat_size() * sizeof(IMAGE_THUNK_DATA);
        hat::memory_protector memory_prot{
            std::bit_cast<uintptr_t>(idd.import_address_table),
            memory_size,
            hat::protection::Read | hat::protection::Write
        };

        callback();
        IATWrite::IatMutex.unlock();
    }
}
