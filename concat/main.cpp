#include <d3d12.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <iostream>

#include "d3dx12.h"

using namespace Microsoft::WRL;

int main()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM library." << std::endl;
        return -1;
    }

    IDXGIFactory6* pFactory = nullptr;
    hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&pFactory));
    if (FAILED(hr)) {
        std::cerr << "Failed to create DXGI factory." << std::endl;
        CoUninitialize();
        return -1;
    }

    IDXGIAdapter1* pAdapter = nullptr;
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &pAdapter); ++adapterIndex) {
        DXGI_ADAPTER_DESC1 adapterDesc;
        pAdapter->GetDesc1(&adapterDesc);

        if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue;
        }

        std::wcout << L"Adapter found: " << adapterDesc.Description << std::endl;
        break;
    }

    if (pAdapter == nullptr) {
        std::cerr << "No suitable adapter found." << std::endl;
        pFactory->Release();
        CoUninitialize();
        return -1;
    }

    ID3D12Device* pDevice = nullptr;
    hr = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice));
    pAdapter->Release();
    if (FAILED(hr)) {
        std::cerr << "Failed to create D3D12 device." << std::endl;
        pFactory->Release();
        CoUninitialize();
        return -1;
    }

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = 1024 * 64;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* pResource = nullptr;
    hr = pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&pResource)
    );

    if (FAILED(hr)) {
        std::cerr << "Failed to create committed resource." << std::endl;
        pDevice->Release();
        pFactory->Release();
        CoUninitialize();
        return -1;
    }

    std::cout << "Resource created successfully." << std::endl;

    pResource->Release();
    pDevice->Release();
    pFactory->Release();
    CoUninitialize();

    return 0;
}
