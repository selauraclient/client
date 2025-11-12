//
// Created by Akashic on 7/1/2025.
//

#ifndef IAT_WRITE_HPP
#define IAT_WRITE_HPP
#include <functional>
#include <mutex>
#include "img_delay_description.hpp"


namespace fi {

    class IATWrite {
    public:
        IATWrite(uintptr_t iat_address, const std::function<void()>& callback, const IDD& idd);
        IATWrite(const IATWrite&) = delete;
        IATWrite& operator=(const IATWrite&) = delete;
        IATWrite(IATWrite&&) = default;

    private:
        static std::mutex IatMutex;
    };
}

#endif //IAT_WRITE_HPP
