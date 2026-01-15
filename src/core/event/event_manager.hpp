#pragma once
#include <pch.hpp>
#include <core/event/event.hpp>

namespace selaura {
    struct event_manager {
        struct listener_entry {
            void* instance;
            void* function;
            std::function<void(selaura::event&)> callback;
        };

        using ListenerMap = std::map<std::type_index, std::vector<listener_entry>>;

        event_manager() {
            data.store(std::make_shared<ListenerMap>());
        }

        template<typename T>
        void subscribe(void(*callback)(T&)) {
            auto type = std::type_index(typeid(T));
            void* fn = reinterpret_cast<void*>(callback);

            auto old_map = data.load();
            while (true) {
                auto new_map = std::make_shared<ListenerMap>(*old_map);

                (*new_map)[type].push_back({
                    nullptr,
                    fn,
                    [callback](selaura::event& e) {
                        callback(static_cast<T&>(e));
                    }
                });

                if (data.compare_exchange_strong(old_map, new_map))
                    break;
            }
        }

        template<typename T>
        void unsubscribe(void(*callback)(T&)) {
            auto type = std::type_index(typeid(T));
            void* fn = reinterpret_cast<void*>(callback);

            auto old_map = data.load();
            while (true) {
                if (!old_map->contains(type))
                    return;

                auto new_map = std::make_shared<ListenerMap>(*old_map);
                auto& entries = (*new_map)[type];

                std::erase_if(entries, [&](const listener_entry& entry) {
                    return entry.instance == nullptr && entry.function == fn;
                });

                if (data.compare_exchange_strong(old_map, new_map))
                    break;
            }
        }

        template<typename Event, typename T>
        void subscribe(T* instance, void (T::*method)(Event&)) {
            auto type = std::type_index(typeid(Event));

            void* fn;
            static_assert(sizeof(method) <= sizeof(void*));
            std::memcpy(&fn, &method, sizeof(method));

            auto old_map = data.load();
            while (true) {
                auto new_map = std::make_shared<ListenerMap>(*old_map);

                (*new_map)[type].push_back({
                    instance,
                    fn,
                    [instance, method](selaura::event& e) {
                        (instance->*method)(static_cast<Event&>(e));
                    }
                });

                if (data.compare_exchange_strong(old_map, new_map))
                    break;
            }
        }

        template<typename Event, typename T>
        void unsubscribe(T* instance, void (T::*method)(Event&)) {
            auto type = std::type_index(typeid(Event));

            void* fn;
            static_assert(sizeof(method) <= sizeof(void*));
            std::memcpy(&fn, &method, sizeof(method));

            auto old_map = data.load();
            while (true) {
                if (!old_map->contains(type))
                    return;

                auto new_map = std::make_shared<ListenerMap>(*old_map);
                auto& entries = (*new_map)[type];

                std::erase_if(entries, [&](const listener_entry& entry) {
                    return entry.instance == instance && entry.function == fn;
                });

                if (data.compare_exchange_strong(old_map, new_map))
                    break;
            }
        }

        template<typename TEvent, typename TLambda>
        void subscribe(TLambda&& callback) {
            auto type = std::type_index(typeid(TEvent));

            auto old_map = data.load();
            while (true) {
                auto new_map = std::make_shared<ListenerMap>(*old_map);

                (*new_map)[type].push_back({
                    nullptr,
                    nullptr,
                    [cb = std::forward<TLambda>(callback)](selaura::event& e) {
                        cb(static_cast<TEvent&>(e));
                    }
                });

                if (data.compare_exchange_strong(old_map, new_map)) break;
            }
        }


        template<typename T>
        void dispatch(T& event) {
            auto current_map = data.load(std::memory_order_acquire);

            auto it = current_map->find(std::type_index(typeid(T)));
            if (it == current_map->end())
                return;

            for (const auto& entry : it->second) {
                entry.callback(event);
            }
        }

        void clear() {
            data.store(std::make_shared<ListenerMap>());
        }

    private:
        std::atomic<std::shared_ptr<ListenerMap>> data;
    };
}
