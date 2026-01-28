#pragma once
#include <pch.hpp>

#include <core/event/event_manager.hpp>
#include <core/service/console.hpp>
#include <core/service/notification_manager.hpp>
#include <core/service/input_manager.hpp>
#include <core/screen/screen_manager.hpp>
#include <core/feature/feature_manager.hpp>

#include <sdk/bedrock/core/MinecraftGame.hpp>

namespace selaura {
    struct service_manager_impl {
        using services_t = std::tuple<
            event_manager,
            console,
            notification_manager,
            input_manager,
            screen_manager,
            feature_manager
        >;

        template <typename T>
        constexpr auto& get() {
            return std::get<T>(services);
        }
    private:
        services_t services;
    };

    inline std::unique_ptr<selaura::service_manager_impl> service_manager;
    inline MinecraftGame* mc;

    template <typename T>
    constexpr auto& get() {
        return service_manager->get<T>();
    }
};