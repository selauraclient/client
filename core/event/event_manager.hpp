#pragma once
#include <pch.hpp>

namespace selaura {
    struct event_manager_impl {
        template<typename T>
        void subscribe(void(*callback)(T&)) {
            std::unique_lock lock(mutex);
            auto type = std::type_index(typeid(T));

            listeners[type].push_back({
                reinterpret_cast<void*>(callback),
                [callback](selaura::event& e) { callback(static_cast<T&>(e)); }
            });
        }

        template<typename T>
        void unsubscribe(void(*callback)(T&)) {
            std::unique_lock lock(mutex);
            auto type = std::type_index(typeid(T));

            if (!listeners.contains(type)) return;

            auto& entries = listeners[type];
            void* target = reinterpret_cast<void*>(callback);

            std::erase_if(entries, [target](const listener_entry& entry) {
                return entry.addr == target;
            });
        }

        template<typename T>
        void dispatch(T& event) {
            std::shared_lock lock(mutex);
            auto type = std::type_index(typeid(T));

            auto it = listeners.find(type);
            if (it != listeners.end()) {
                for (const auto& entry : it->second) {
                    entry.callback(event);
                }
            }
        }

        void clear() {
            std::unique_lock lock(mutex);
            listeners.clear();
        }

    private:
        struct listener_entry {
            void* addr;
            std::function<void(selaura::event&)> callback;
        };

        std::map<std::type_index, std::vector<listener_entry>> listeners;
        mutable std::shared_mutex mutex;
    };

    inline std::unique_ptr<event_manager_impl> event_manager;
};