#pragma once
#include <pch.hpp>

struct MinecraftPackets {
    MCAPI static std::shared_ptr<Packet> createPacket(MinecraftPacketIds packetId);
};