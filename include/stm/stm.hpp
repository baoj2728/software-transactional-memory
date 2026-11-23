#pragma once

#include "stm/atomic.hpp"

#include <functional>
#include <mutex>
#include <stdexcept>
#include <type_traits>

namespace stm {

template <typename Fn>
void atomic_run(Fn&& body) {
    static_assert(std::is_void_v<std::invoke_result_t<Fn&>>,
                  "initial atomic_run supports void callables only");

    if (detail::active_transaction != nullptr) {
        throw std::logic_error("nested STM transactions are not supported");
    }

    while (true) {
        detail::TransactionContext tx;
        tx.begin();
        detail::ActiveTransactionGuard scope(tx);
        try {
            std::invoke(body);
            if (tx.commit()) return;
            tx.abort();
        } catch (const detail::TransactionConflict&) {
            tx.abort();
        } catch (...) {
            tx.abort();
            throw;
        }
    }
}

}  // namespace stm
