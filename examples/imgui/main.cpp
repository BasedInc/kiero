#include <kiero.hpp>

#ifndef KIERO_USE_MINHOOK
# error "The example requires that minhook be enabled!"
#endif

#include <Windows.h>

#include <chrono>
#include <condition_variable>
#include <format>
#include <mutex>

#include <imgui.h>

#include "impl/renderer_impl.h"
#include "impl/shared.h"

static std::condition_variable runningCv;
static std::mutex runningMutex;
static bool isRunning{};

void impl::showExampleWindow(std::string_view comment) {
    const auto title = std::format("Kiero Dear ImGui Example ({})", comment);

    ImGui::Begin(title.c_str());
    {
        ImGui::Text("Hello");
        ImGui::Button("World!");
    }
    ImGui::End();

    const auto& io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyDown(ImGuiKey_PageUp)) {
        {
            std::lock_guard g{runningMutex};
            isRunning = false;
        }
        runningCv.notify_all();
    }
}

static void(*initHooks())() {
    switch (kiero::getRenderType()) {
        using enum kiero::RenderType;
        case D3D9:
            impl::d3d9::init();
            return &impl::d3d9::shutdown;
        case D3D10:
            impl::d3d10::init();
            return &impl::d3d10::shutdown;
        case D3D11:
            impl::d3d11::init();
            return &impl::d3d11::shutdown;
        case D3D12:
            impl::d3d12::init();
            return &impl::d3d12::shutdown;
        case OpenGL:
            impl::opengl::init();
            return &impl::opengl::shutdown;
        case Vulkan:
            // TODO: Vulkan implementation?
        case None:
        case Auto:
            break;
    }
    return nullptr;
}

static DWORD WINAPI kieroInitThread(const LPVOID hModule) {
    if (kiero::init(kiero::RenderType::Auto) == kiero::Status::Success) {
        const auto shutdown = initHooks();

        isRunning = true;
        {
            std::unique_lock lock{runningMutex};
            runningCv.wait(lock, [] { return !isRunning; });
        }

        kiero::shutdown();

        // Hooks won't get called after kiero::shutdown, but addresses to detours could still exist on the return stack
        // Instead of an elegant synchronization solution, wait 200 ms
        std::this_thread::sleep_for(std::chrono::milliseconds{200});

        // Since the previous assumption is being made, it's *fine* to just de-initialize ImGui and other backend
        // related vars from this thread now...
        if (shutdown) {
            shutdown();
        }
    }

    FreeLibraryAndExitThread(static_cast<HMODULE>(hModule), 0);
}

BOOL WINAPI DllMain(const HINSTANCE hInstance, const DWORD fdwReason, [[maybe_unused]] LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
#ifdef _DLL
        DisableThreadLibraryCalls(hInstance);
#endif
        const auto thread = CreateThread(nullptr, 0, &kieroInitThread, hInstance, 0, nullptr);
        if (!thread) {
            return FALSE;
        }
        CloseHandle(thread);
    }
    return TRUE;
}
