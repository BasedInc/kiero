#ifdef KIERO_INCLUDE_D3D9

#include "d3d9_impl.h"
#include <d3d9.h>
#include <assert.h>

#include "win32_impl.h"
#include "shared.h"

#include <kiero.hpp>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx9.h>

using Reset = long(__stdcall*)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
static Reset oReset = nullptr;

using EndScene = long(__stdcall*)(LPDIRECT3DDEVICE9);
static EndScene oEndScene = nullptr;

long __stdcall hkReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
	ImGui_ImplDX9_InvalidateDeviceObjects();
	long result = oReset(pDevice, pPresentationParameters);
	ImGui_ImplDX9_CreateDeviceObjects();

	return result;
}

long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice) {
	static bool init = false;

	if (!init) {
		D3DDEVICE_CREATION_PARAMETERS params;
		pDevice->GetCreationParameters(&params);

		impl::win32::init(params.hFocusWindow);

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(params.hFocusWindow);
		ImGui_ImplDX9_Init(pDevice);

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

#endif // KIERO_INCLUDE_D3D9
