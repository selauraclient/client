#pragma once
#include <pch.hpp>
#include "RakNet.hpp"

namespace RakNet {
    struct RakPeer {
        MCAPI int GetAveragePing(const RakNet::AddressOrGUID systemIdentifier);
    };
};