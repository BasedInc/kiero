#include <kiero.hpp>

#include <set>

#include <MinHook.h>

namespace {
    std::set<void*> g_hooks;
    bool uninit = false;
}

namespace kiero::hook {

    Status init() {
        const auto status = MH_Initialize();
        if (status == MH_ERROR_ALREADY_INITIALIZED) {
            uninit = false;
        } else if (status != MH_OK) {
            return Status::UnknownError;
        } else {
            uninit = true;
        }
        return Status::Success;
    }

    void shutdown() {
        if (uninit) {
            MH_Uninitialize();
        } else {
            for (void* target : g_hooks) {
                MH_QueueDisableHook(target);
            }
            MH_ApplyQueued();
            for (void* target : g_hooks) {
                MH_RemoveHook(target);
            }
        }
        g_hooks.clear();
    }

    Status bind(void* target, void** original, void* detour) {
        if (g_hooks.contains(target)) {
            return Status::MethodAlreadyBoundError;
        }
        if (MH_CreateHook(target, detour, original) != MH_OK || MH_EnableHook(target) != MH_OK) {
            return Status::UnknownError;
        }
        g_hooks.insert(target);
        return Status::Success;
    }

    Status unbind(void* target) {
        if (!g_hooks.contains(target)) {
            return Status::MethodNotBoundError;
        }
        if (MH_RemoveHook(target) != MH_OK) {
            return Status::UnknownError;
        }
        g_hooks.erase(target);
        return Status::Success;
    }
}
