#pragma once
#include <pch.hpp>

namespace selaura {
    namespace detail {
        inline std::vector<void(*)()> cleanup_tasks;

        inline void register_for_cleanup(void(*fn)()) {
            cleanup_tasks.push_back(fn);
        }

        inline void run_cleanup() {
            for (auto& task : cleanup_tasks) task();
            cleanup_tasks.clear();
        }
    }
}