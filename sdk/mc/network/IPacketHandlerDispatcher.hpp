//
// Created by chyves on 11/12/2025.
//

#pragma once
#include <pch.hpp>

class Packet;

class IPacketHandlerDispatcher {
public:
    virtual ~IPacketHandlerDispatcher() = default;
    virtual void handle(void* networkIdentifier, void* netEventCallback, std::shared_ptr<Packet>&) const = 0;
};