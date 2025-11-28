#include "win32_impl.h"

#include <Windows.h>

#include <kiero.hpp>

#include <imgui.h>
#include <imgui_impl_win32.h>

static WNDPROC oWndProc = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK hkWindowProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam) > 0)
		return 1L;

	return CallWindowProcA(oWndProc, hwnd, uMsg, wParam, lParam);
}

void impl::win32::init(void* hwnd) {
	oWndProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(static_cast<HWND>(hwnd), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&hkWindowProc)));
}
