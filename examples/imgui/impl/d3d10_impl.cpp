#include "renderer_impl.h"

#ifdef KIERO_INCLUDE_D3D10

#include <cassert>
#include <d3d10.h>

#include <winrt/base.h>

#include "win32_impl.h"

#include <kiero.hpp>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx10.h>

using Present = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);
static Present oPresent = nullptr;

static HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, const UINT SyncInterval, const UINT Flags) {
	static bool init = false;

	if (!init) {
		DXGI_SWAP_CHAIN_DESC desc;
		winrt::check_hresult(pSwapChain->GetDesc(&desc));

		winrt::com_ptr<ID3D10Device> device;
		winrt::check_hresult(pSwapChain->GetDevice(IID_PPV_ARGS(device.put())));

		ImGui::CreateContext();
		ImGui_ImplDX10_Init(device.get());
		impl::win32::init(desc.OutputWindow);

		init = true;
	}

	ImGui_ImplDX10_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	impl::showExampleWindow("D3D10");

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX10_RenderDrawData(ImGui::GetDrawData());

	return oPresent(pSwapChain, SyncInterval, Flags);
}

void impl::d3d10::init() {
	const auto status = kiero::bind<&IDXGISwapChain::Present>(&oPresent, &hkPresent);
	assert(status == kiero::Status::Success);
}

void impl::d3d10::shutdown() {
	if (!ImGui::GetCurrentContext()) {
		return;
	}
    win32::shutdown();
	ImGui_ImplDX10_Shutdown();
	ImGui::DestroyContext();
}

#else

void impl::d3d10::init() {}
void impl::d3d10::shutdown() {}

#endif // KIERO_INCLUDE_D3D10
