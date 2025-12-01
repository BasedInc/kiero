#include <kiero.hpp>

#include <cassert>
#include <set>

#include <safetyhook/easy.hpp>

namespace {
    struct HookComparator {
        using is_transparent = void;

        bool operator()(const safetyhook::InlineHook& lhs, const safetyhook::InlineHook& rhs) const noexcept {
            return lhs.target() < rhs.target();
        }

        bool operator()(const safetyhook::InlineHook& lhs, const void* target) const noexcept {
            return lhs.target() < target;
        }

        bool operator()(const void* target, const safetyhook::InlineHook& rhs) const noexcept {
            return target < rhs.target();
        }
    };

    std::set<safetyhook::InlineHook, HookComparator> g_hooks;
}

namespace kiero::hook {

    Status init() {
        return Status::Success;
    }

    void shutdown() {
        g_hooks.clear();
    }

    Status bind(void* target, void** original, void* detour) {
        if (g_hooks.contains(target)) {
            return Status::MethodAlreadyBoundError;
        }

        auto hook = safetyhook::create_inline(target, detour);
        if (!hook) {
            return Status::UnknownError;
        }

        auto [it, inserted] = g_hooks.insert(std::move(hook));
        assert(inserted);
        *original = it->original<void*>();

        return Status::Success;
    }

    Status unbind(void* target) {
        const auto it = g_hooks.find(target);
        if (it == g_hooks.end()) {
            return Status::MethodNotBoundError;
        }
        g_hooks.erase(it);
        return Status::Success;
    }
}
