//
// Created by Akashic on 7/1/2025.
//

#ifndef RVA_PTR_HPP
#define RVA_PTR_HPP
#include <cstddef>

#include "basic.hpp"

namespace fi {

    template <typename T> class RVAPtr {
    private:
        static T *resolve_ptr(const RVA rva) {
            return reinterpret_cast<T *>(
                reinterpret_cast< uint8_t *>(&__ImageBase) + rva);
        }

    public:
        RVAPtr(const RVA rva) : resolved(resolve_ptr(rva)) {}
        RVAPtr(std::nullptr_t) : resolved(nullptr) {}
        RVAPtr() = default;

        T *operator->() { return this->resolved; }

        T *operator->() const { return this->resolved; }

        T &operator*() { return *this->resolved; }

        T &operator*() const { return *this->resolved; }

        operator T *() { return this->resolved; }

        operator T *() const { return this->resolved; }

        bool operator==(const RVAPtr<T> other) const {
            return other.resolved == this->resolved;
        }

        bool operator==(T *other) const { return other == this->resolved; }

        bool operator!=(const RVAPtr<T> other) const { return !(*this == other); }

        bool operator!=(T *other) const { return !(*this == other); }

        T *get() const { return this->resolved; }

        T operator[](size_t index) { return this->resolved[index]; }

        RVAPtr operator++() {
            const auto ptr = *this;
            ++this->resolved;
            return ptr;
        }

        RVAPtr operator++(int) {
            const auto ptr = *this;
            ++this->resolved;
            return ptr;
        }

    private:
        T *resolved{nullptr};
    };

    using RVAString = RVAPtr<const char>;

} // fi

#endif //RVA_PTR_HPP
