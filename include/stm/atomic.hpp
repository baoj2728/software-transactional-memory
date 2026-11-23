#pragma once

#include "stm/detail/context.hpp"

#include <atomic>
#include <stdexcept>
#include <type_traits>

namespace stm {

template <typename T>
class atomic final : public detail::VariableBase {
    static_assert(std::is_trivially_copyable_v<T>,
                  "stm::atomic<T> requires a trivially copyable T");

public:
    explicit atomic(T initial) : value_(initial) {}

    atomic(const atomic&) = delete;
    atomic& operator=(const atomic&) = delete;

    T load() const {
        if (detail::active_transaction == nullptr) {
            throw std::logic_error("stm::atomic::load requires atomic_run");
        }
        return detail::active_transaction->read(*this);
    }

    void store(T value) {
        if (detail::active_transaction == nullptr) {
            throw std::logic_error("stm::atomic::store requires atomic_run");
        }
        detail::active_transaction->write(*this, value);
    }

    operator T() const {
        return load();
    }

    atomic& operator=(T value) {
        store(value);
        return *this;
    }

    atomic& operator+=(T value) {
        store(load() + value);
        return *this;
    }

    atomic& operator-=(T value) {
        store(load() - value);
        return *this;
    }


private:
    friend class detail::TransactionContext;
    std::atomic<T> value_;
};

template <typename T>
T detail::TransactionContext::read(const stm::atomic<T>& variable) {
    const auto it = write_set_.find(&variable);
    if (it != write_set_.end()) {
        return std::any_cast<T>(it->second.buffered_value);
    }

    uint64_t before = variable.version_lock_.load();

    if (before & 1ULL) {
        throw detail::TransactionConflict{};
    }

    if ((before >> 1) > start_version_) {
        throw detail::TransactionConflict{};
    }
    
    const T value = variable.value_.load();
    uint64_t after = variable.version_lock_.load();

    if (before != after) {
        throw detail::TransactionConflict{};
    }

    auto [read_it, inserted] = read_set_.try_emplace(&variable, before >> 1);

    if (!inserted && read_it->second != (before >> 1)) {
        throw detail::TransactionConflict{};
    }

    return value;
}

template <typename T>
void detail::TransactionContext::write(stm::atomic<T>& variable, T value) {
    write_set_.insert_or_assign(
        &variable,
        detail::PendingWrite{
            &variable,
            std::any(value),
            [&variable, value]() noexcept { variable.value_.store(value); },
            false
        }
    );
}

}  // namespace stm
