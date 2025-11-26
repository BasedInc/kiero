#pragma once

#include <dxgi.h>
#include <tchar.h>

namespace kiero::d3d {

	class TemporaryWindow {
	public:
		TemporaryWindow() {
			this->windowClass.cbSize = sizeof(WNDCLASSEX);
			this->windowClass.style = CS_HREDRAW | CS_VREDRAW;
			this->windowClass.lpfnWndProc = DefWindowProc;
			this->windowClass.cbClsExtra = 0;
			this->windowClass.cbWndExtra = 0;
			this->windowClass.hInstance = GetModuleHandle(nullptr);
			this->windowClass.hIcon = nullptr;
			this->windowClass.hCursor = nullptr;
			this->windowClass.hbrBackground = nullptr;
			this->windowClass.lpszMenuName = nullptr;
			this->windowClass.lpszClassName = _T("Kiero");
			this->windowClass.hIconSm = nullptr;
			::RegisterClassEx(&this->windowClass);

			this->handle = ::CreateWindow(
				this->windowClass.lpszClassName,
				_T("Kiero DirectX Window"),
				WS_OVERLAPPEDWINDOW,
				0, 0, 100, 100,
				nullptr,
				nullptr,
				this->windowClass.hInstance,
				nullptr
			);
		}

		~TemporaryWindow() {
			::DestroyWindow(this->handle);
            ::UnregisterClass(this->windowClass.lpszClassName, this->windowClass.hInstance);
		}

		explicit operator HWND() const {
			return this->handle;
		}

	private:
		WNDCLASSEX windowClass{};
		HWND handle{};
	};

	static TemporaryWindow createTempWindow() {
		return TemporaryWindow{};
	}

	static DXGI_SWAP_CHAIN_DESC createSwapChainDesc(const HWND window) {
		constexpr DXGI_RATIONAL refreshRate {
			.Numerator = 60,
			.Denominator = 1,
		};

        constexpr DXGI_MODE_DESC bufferDesc {
        	.Width = 100,
        	.Height = 100,
        	.RefreshRate = refreshRate,
        	.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        	.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
        	.Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
        };

        constexpr DXGI_SAMPLE_DESC sampleDesc {
        	.Count = 1,
        	.Quality = 0,
        };

        return DXGI_SWAP_CHAIN_DESC {
        	.BufferDesc = bufferDesc,
        	.SampleDesc = sampleDesc,
        	.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        	.BufferCount = 2,
        	.OutputWindow = window,
        	.Windowed = 1,
        	.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        	.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
        };
	}
}
