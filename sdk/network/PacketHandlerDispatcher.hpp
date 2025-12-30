#pragma once
#include <pch.hpp>

template <MinecraftPacketIds id, bool Unknown = false>
struct PacketHandlerDispatcherInstance : public IPacketHandlerDispatcher {
    virtual void handle(void* networkIdentifier, void* netEventCallback, std::shared_ptr<Packet>&) const = 0;
};