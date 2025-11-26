#pragma once

#include "kiero_d3dcommon.hpp"

#include <d3d12.h>
#include <winrt/base.h>

namespace kiero {

    template<>
    inline Status initRenderType<RenderType::D3D12>() {
        using PFN_CREATE_DXGI_FACTORY = decltype(&CreateDXGIFactory);

        const HMODULE libDXGI = ::GetModuleHandle(_T("dxgi.dll"));
        const HMODULE libD3D12 = ::GetModuleHandle(_T("d3d12.dll"));
        if (libDXGI == nullptr || libD3D12 == nullptr) {
            return Status::ModuleNotFoundError;
        }

        const auto CreateDXGIFactory = reinterpret_cast<PFN_CREATE_DXGI_FACTORY>(
            GetProcAddress(libDXGI, "CreateDXGIFactory"));
        if (CreateDXGIFactory == nullptr) {
            return Status::UnknownError;
        }

        winrt::com_ptr<IDXGIFactory> factory;
        if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(factory.put())))) {
            return Status::UnknownError;
        }

        const auto D3D12CreateDevice = reinterpret_cast<PFN_D3D12_CREATE_DEVICE>(
            GetProcAddress(libD3D12, "D3D12CreateDevice"));
        if (D3D12CreateDevice == nullptr) {
            return Status::UnknownError;
        }

        winrt::com_ptr<ID3D12Device> device;
        if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.put())))) {
            return Status::UnknownError;
        }

        constexpr D3D12_COMMAND_QUEUE_DESC queueDesc{
            .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Priority = 0,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
            .NodeMask = 0,
        };

        winrt::com_ptr<ID3D12CommandQueue> commandQueue;
        if (FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(commandQueue.put())))) {
            return Status::UnknownError;
        }

        winrt::com_ptr<ID3D12CommandAllocator> commandAllocator;
        if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator.put())))) {
            return Status::UnknownError;
        }

        winrt::com_ptr<ID3D12GraphicsCommandList> commandList;
        if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.get(), nullptr, IID_PPV_ARGS(commandList.put())))) {
            return Status::UnknownError;
        }

        const auto window = d3d::createTempWindow();
        DXGI_SWAP_CHAIN_DESC swapChainDesc = d3d::createSwapChainDesc(static_cast<HWND>(window));

        winrt::com_ptr<IDXGISwapChain> swapChain;
        if (FAILED(factory->CreateSwapChain(commandQueue.get(), &swapChainDesc, swapChain.put()))) {
            return Status::UnknownError;
        }

        add_vtable<IDXGISwapChain1, IDXGISwapChain2, IDXGISwapChain3, IDXGISwapChain4>(swapChain);
        add_vtable<ID3D12Device1, ID3D12Device2, ID3D12Device3, ID3D12Device4, ID3D12Device5, ID3D12Device6,
            ID3D12Device7, ID3D12Device8, ID3D12Device9, ID3D12Device10, ID3D12Device11, ID3D12Device12, ID3D12Device13,
            ID3D12Device14>(device);
        add_vtable(commandQueue);
        add_vtable(commandAllocator);
        add_vtable<ID3D12GraphicsCommandList1, ID3D12GraphicsCommandList2, ID3D12GraphicsCommandList3,
            ID3D12GraphicsCommandList4, ID3D12GraphicsCommandList5, ID3D12GraphicsCommandList6,
            ID3D12GraphicsCommandList7, ID3D12GraphicsCommandList8, ID3D12GraphicsCommandList9,
            ID3D12GraphicsCommandList10>(commandList);

        return Status::Success;
    }
}
