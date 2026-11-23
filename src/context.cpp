#include "stm/detail/context.hpp"

#include<atomic>
#include <stdexcept>

namespace stm::detail {

thread_local TransactionContext* active_transaction = nullptr;

std::mutex& fallback_mutex() {
    static std::mutex mutex;
    return mutex;
}

std::atomic<std::uint64_t>& global_version_clock() {
    static std::atomic<std::uint64_t> clock{0};
    return clock;
}

ActiveTransactionGuard::ActiveTransactionGuard(TransactionContext& tx) {
    if (active_transaction != nullptr) {
        throw std::logic_error("nested STM transactions are not supported");
    }
    active_transaction = &tx;
}

ActiveTransactionGuard::~ActiveTransactionGuard() {
    active_transaction = nullptr;
}

void TransactionContext::begin() {
    abort();
    start_version_ = global_version_clock().load();
}

bool TransactionContext::commit() {
    auto release_locks = [&]() {
        for (auto& [key, value] : write_set_) {
            if (value.lock_acquired) {
                value.lock_acquired = false;
                value.target->version_lock_.fetch_and(~uint64_t{1});
            }
        }
    };

    for (auto& [address, entry] : write_set_) {
        if (!entry.publish) {
            release_locks();
            throw std::logic_error("Missing publish function");
        }
        uint64_t expected = entry.target->version_lock_.load();

        if (expected & 1ULL) {
            release_locks();
            return false;
        }

        if (!entry.target->version_lock_.compare_exchange_strong(expected, expected | 1ULL)) {
            release_locks();
            return false;
        }

        entry.lock_acquired = true;
    }

    const uint64_t commit_version = write_set_.empty() ? 0 : global_version_clock().fetch_add(1) + 1;
    
    for (auto& [address, old_version] : read_set_) {
        std::uint64_t state = address->version_lock_.load();
        std::uint64_t new_version = state >> 1;
        bool locked = state & 1ULL;
        bool locked_by_us = write_set_.find(address) != write_set_.end();

        if (new_version != old_version || (locked && !locked_by_us)) {
            release_locks();
            return false;
        }
    }

    for (auto& [address, entry] : write_set_) {
        entry.publish();
    }
    for (auto& [address, entry] : write_set_) {
        entry.target->version_lock_.store(commit_version << 1);
        entry.lock_acquired = false;
    }
    
    abort();
    return true;
}

void TransactionContext::abort() noexcept {
    auto release_locks = [&]() {
        for (auto& [key, value] : write_set_) {
            if (value.lock_acquired) {
                value.lock_acquired = false;
                value.target->version_lock_.fetch_and(~uint64_t{1});
            }
        }
    };
    release_locks();
    read_set_.clear();
    write_set_.clear();
    start_version_ = 0;
}

}  // namespace stm::detail
