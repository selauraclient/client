//
// Created by chyves on 11/12/2025.
//

#pragma once
#include <pch.hpp>

struct MinecraftPackets {
    MCAPI static std::shared_ptr<Packet> createPacket(MinecraftPacketIds packetId);
};