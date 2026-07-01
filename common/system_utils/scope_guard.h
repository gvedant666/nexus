#pragma once
#include <utility>

template <typename Callable>
class ScopeGuard {
public:
    explicit ScopeGuard(Callable&& fn) 
        : cleanup_fn_(std::move(fn)), active_(true) {}
    
    // Prevent copying to ensure cleanup only happens once
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    // Allow moving
    ScopeGuard(ScopeGuard&& other) noexcept 
        : cleanup_fn_(std::move(other.cleanup_fn_)), active_(other.active_) {
        other.active_ = false;
    }

    ~ScopeGuard() {
        if (active_) {
            try {
                cleanup_fn_();
            } catch (...) {
                // Destructors must never throw in C++
            }
        }
    }

    // Call this when initialization succeeds to cancel the rollback
    void dismiss() noexcept {
        active_ = false;
    }

private:
    Callable cleanup_fn_;
    bool active_;
};

// Deduction guide/helper for easier syntax
template <typename Callable>
ScopeGuard<Callable> make_scope_guard(Callable&& fn) {
    return ScopeGuard<Callable>(std::forward<Callable>(fn));
}