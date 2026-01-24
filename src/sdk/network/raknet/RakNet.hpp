#pragma once
#include <pch.hpp>

namespace RakNet {
    struct AddressOrGUID {
        void* rakNetGuid;
        void* systemAddress;
    };
};