#pragma once

#include <any>
#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>

namespace stm {
template <typename T>
class atomic;

namespace detail {

// All transactional variables share the same synchronization metadata.
// In the TL2 implementation, bit 0 will be the lock bit and bits 1..63
// will be the last committed version.
class VariableBase {
public:
    VariableBase() = default;
    VariableBase(const VariableBase&) = delete;
    VariableBase& operator=(const VariableBase&) = delete;

private:
    friend class TransactionContext;
    std::atomic<std::uint64_t> version_lock_{0};
};

struct PendingWrite {
    VariableBase* target;
    std::any buffered_value;
    std::function<void()> publish;

    bool lock_acquired = false;
};

struct TransactionConflict {};

class TransactionContext {
public:
    void begin();

    template <typename T>
    T read(const stm::atomic<T>& variable);

    template <typename T>
    void write(stm::atomic<T>& variable, T value);

    bool commit();
    void abort() noexcept;

private:
    std::uint64_t start_version_{0};
    std::unordered_map<const VariableBase*, std::uint64_t> read_set_;
    std::unordered_map<const VariableBase*, PendingWrite> write_set_;
};

// The pointer is independent for each OS thread.
extern thread_local TransactionContext* active_transaction;

// Used only by the first, coarse-grained-lock implementation.
std::mutex& fallback_mutex();
std::atomic<std::uint64_t>& global_version_clock();

class ActiveTransactionGuard {
public:
    explicit ActiveTransactionGuard(TransactionContext& tx);
    ~ActiveTransactionGuard();

    ActiveTransactionGuard(const ActiveTransactionGuard&) = delete;
    ActiveTransactionGuard& operator=(const ActiveTransactionGuard&) = delete;
};

}  // namespace detail
}  // namespace stm
