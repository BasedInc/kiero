#include "renderer_impl.h"

#ifdef KIERO_INCLUDE_OPENGL

#include <Windows.h>

#include "win32_impl.h"
#include "shared.h"

#include <kiero.hpp>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_opengl3.h>

using wglSwapBuffers = BOOL(WINAPI*)(HDC);
static wglSwapBuffers owglSwapBuffers{};

static BOOL WINAPI hk_wglSwapBuffers(const HDC hDc) {
    static bool init = false;

    if (!init) {
        const HWND hwnd = WindowFromDC(hDc);

        ImGui::CreateContext();
        ImGui_ImplWin32_InitForOpenGL(hwnd);
        ImGui_ImplOpenGL3_Init();

		impl::win32::init(hwnd);

        init = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    impl::showExampleWindow("OpenGL");

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return owglSwapBuffers(hDc);
}

void impl::opengl::init() {
    kiero::bind(336, reinterpret_cast<void**>(&owglSwapBuffers), &hk_wglSwapBuffers);
}

#else

void impl::opengl::init() {}

#endif // KIERO_INCLUDE_OPENGL
