#pragma once
#include <pch.hpp>
#include <core/event/event.hpp>

namespace selaura {
    struct event_manager {
        struct listener_entry {
            void* addr;
            std::function<void(selaura::event&)> callback;
        };

        using ListenerMap = std::map<std::type_index, std::vector<listener_entry>>;

        event_manager() {
            data.store(std::make_shared<ListenerMap>());
        }

        template<typename T>
        void subscribe(void(*callback)(T&)) {
            auto type = std::type_index(typeid(T));
            void* target = reinterpret_cast<void*>(callback);

            auto old_map = data.load();
            while (true) {
                auto new_map = std::make_shared<ListenerMap>(*old_map);

                (*new_map)[type].push_back({
                    target,
                    [callback](selaura::event& e) { callback(static_cast<T&>(e)); }
                });

                if (data.compare_exchange_strong(old_map, new_map)) {
                    break;
                }
            }
        }

        template<typename T>
        void unsubscribe(void(*callback)(T&)) {
            auto type = std::type_index(typeid(T));
            void* target = reinterpret_cast<void*>(callback);

            auto old_map = data.load();
            while (true) {
                if (!old_map->contains(type)) return;

                auto new_map = std::make_shared<ListenerMap>(*old_map);
                auto& entries = (*new_map)[type];

                std::erase_if(entries, [target](const listener_entry& entry) {
                    return entry.addr == target;
                });

                if (data.compare_exchange_strong(old_map, new_map)) {
                    break;
                }
            }
        }

        template<typename T>
        void dispatch(T& event) {
            auto current_map = data.load(std::memory_order_acquire);

            auto it = current_map->find(std::type_index(typeid(T)));
            if (it != current_map->end()) {
                for (const auto& entry : it->second) {
                    entry.callback(event);
                }
            }
        }

        void clear() {
            data.store(std::make_shared<ListenerMap>());
        }

    private:
        std::atomic<std::shared_ptr<ListenerMap>> data;
    };
};