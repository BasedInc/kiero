#include <kiero.hpp>

#include <vector>

#include <MinHook.h>

namespace {
    std::vector<void*> g_hooks;
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
        }
        g_hooks.clear();
    }

    Status bind(void* target, void** original, void* detour) {
        if (MH_CreateHook(target, detour, original) != MH_OK || MH_EnableHook(target) != MH_OK) {
            return Status::UnknownError;
        }

        g_hooks.emplace_back(target);
        return Status::Success;
    }

    Status unbind(void* target) {
        MH_DisableHook(target);
        return Status::Success;
    }
}
