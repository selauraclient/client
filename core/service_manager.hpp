#pragma once
#include <pch.hpp>

#include <core/event/event_manager.hpp>
#include <core/service/console.hpp>
#include <core/service/notification_manager.hpp>
#include <core/renderer/renderer.hpp>

namespace selaura {
    struct service_manager_impl {
        using services_t = std::tuple<
            event_manager,
            console,
            notification_manager,
            renderer
        >;

        template <typename T>
        constexpr auto& get() {
            return std::get<T>(services);
        }
    private:
        services_t services;
    };

    inline std::unique_ptr<selaura::service_manager_impl> service_manager;

    template <typename T>
    constexpr auto& get() {
        return service_manager->get<T>();
    }
};