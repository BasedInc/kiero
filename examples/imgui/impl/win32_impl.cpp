#include "win32_impl.h"

#include <Windows.h>

#include <kiero.hpp>

#include <imgui.h>
#include <imgui_impl_win32.h>

static HWND g_Hwnd = nullptr;
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
	g_Hwnd = static_cast<HWND>(hwnd);
	oWndProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(g_Hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&hkWindowProc)));
}

void impl::win32::shutdown() {
	if (oWndProc) {
		SetWindowLongPtr(g_Hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(oWndProc));
	}
}
