#include "renderer_impl.h"

#ifdef KIERO_INCLUDE_D3D9

#include <d3d9.h>
#include <assert.h>

#include <winrt/base.h>

#include "win32_impl.h"
#include "shared.h"

#include <kiero.hpp>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx9.h>

using Reset = HRESULT(__stdcall*)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
static Reset oReset = nullptr;

using EndScene = HRESULT(__stdcall*)(LPDIRECT3DDEVICE9);
static EndScene oEndScene = nullptr;

HRESULT __stdcall hkReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
	ImGui_ImplDX9_InvalidateDeviceObjects();
	const auto result = oReset(pDevice, pPresentationParameters);
	ImGui_ImplDX9_CreateDeviceObjects();
	return result;
}

HRESULT __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice) {
	static bool init = false;

	if (!init) {
		D3DDEVICE_CREATION_PARAMETERS params;
		winrt::check_hresult(pDevice->GetCreationParameters(&params));

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(params.hFocusWindow);
		ImGui_ImplDX9_Init(pDevice);

		impl::win32::init(params.hFocusWindow);

		init = true;
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	impl::showExampleWindow("D3D9");

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return oEndScene(pDevice);
}

void impl::d3d9::init() {
	auto status = kiero::bind<&IDirect3DDevice9::Reset>(&oReset, &hkReset);
	assert(status == kiero::Status::Success);
	status = kiero::bind<&IDirect3DDevice9::EndScene>(&oEndScene, &hkEndScene);
	assert(status == kiero::Status::Success);
}

#else

void impl::d3d9::init() {}

#endif // KIERO_INCLUDE_D3D9
