#pragma once
#include <pch.hpp>

class Packet;

class IPacketHandlerDispatcher {
public:
    virtual ~IPacketHandlerDispatcher() = default;
    virtual void handle(void* networkIdentifier, void* netEventCallback, std::shared_ptr<Packet>&) const = 0;
};