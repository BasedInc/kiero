#include "d3d11_impl.h"

#ifdef KIERO_INCLUDE_D3D11

#include <cassert>
#include <d3d11.h>

#include <winrt/base.h>

#include "win32_impl.h"
#include "shared.h"

#include <kiero.hpp>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

using Present = long(__stdcall*)(IDXGISwapChain*, UINT, UINT);
static Present oPresent = nullptr;

static long __stdcall hkPresent(IDXGISwapChain* pSwapChain, const UINT SyncInterval, const UINT Flags) {
	static bool init = false;

	if (!init) {
		DXGI_SWAP_CHAIN_DESC desc;
		pSwapChain->GetDesc(&desc);

		winrt::com_ptr<ID3D11Device> device;
		pSwapChain->GetDevice(IID_PPV_ARGS(device.put()));

		winrt::com_ptr<ID3D11DeviceContext> context;
		device->GetImmediateContext(context.put());

		impl::win32::init(desc.OutputWindow);

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(desc.OutputWindow);
		ImGui_ImplDX11_Init(device.get(), context.get());

		init = true;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	impl::showExampleWindow("D3D11");

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return oPresent(pSwapChain, SyncInterval, Flags);
}

void impl::d3d11::init() {
	const auto status = kiero::bind<&IDXGISwapChain::Present>(&oPresent, &hkPresent);
	assert(status == kiero::Status::Success);
}

#endif // KIERO_INCLUDE_D3D11
