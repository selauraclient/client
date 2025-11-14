//
// Created by chyves on 11/12/2025.
//

#pragma once
#include <pch.hpp>

enum class MinecraftPacketIds : int {
    Text = 9
};

enum class PacketPriority : int {
    ImmediatePriority = 0,
    HighPriority = 1,
    MediumPriority = 2,
    LowPriority = 3,
    NumberOfPriorities = 4,
};

namespace NetworkPeer {
    enum class Reliability : int {
        Reliable = 0,
        ReliableOrdered = 1,
        Unreliable = 2,
        UnreliableSequenced = 3,
    };
};

enum class Compressibility : int {
    Compressible = 0,
    Incompressible = 1,
};

class Packet {
public:
    PacketPriority mPriority;
    NetworkPeer::Reliability mReliability;
    uint8_t mClientSubId;
    bool mIsHandled;
    std::chrono::steady_clock::time_point mRecieveTimepoint;
    const IPacketHandlerDispatcher* mHandler;
    Compressibility mCompressible;
public:
    virtual ~Packet() = default;
    virtual MinecraftPacketIds getId() const = 0;
    virtual std::string getName() const = 0;
};