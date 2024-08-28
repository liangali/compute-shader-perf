#include <fstream>
#include <iostream>
#include <array>
#include <vector>

#include "d3dx12.h"
#include <wrl.h>
#include "dxgi1_4.h"
#include "d3dcompiler.h"


using Microsoft::WRL::ComPtr;

struct Constants
{
  int StartIndex;
  int EndIndex;
  int Stride;
};

inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

#define SAFE_RELEASE(p) if (p) (p)->Release()

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}

void ResourceBarrier(_In_ ID3D12GraphicsCommandList* pCmdList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After, D3D12_RESOURCE_BARRIER_FLAGS Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
{
    D3D12_RESOURCE_BARRIER barrierDesc = {};

    barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrierDesc.Flags = Flags;
    barrierDesc.Transition.pResource = pResource;
    barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrierDesc.Transition.StateBefore = Before;
    barrierDesc.Transition.StateAfter = After;

    pCmdList->ResourceBarrier(1, &barrierDesc);
}

void PrintAdapterInfo(IDXGIAdapter1* adapter) {
    DXGI_ADAPTER_DESC1 adapterDescriptor;
    adapter->GetDesc1(&adapterDescriptor);
    wprintf(
        L"Device: %s (VendorID: 0x%04x DeviceID: 0x%04x)\n",
        adapterDescriptor.Description, adapterDescriptor.VendorId, adapterDescriptor.DeviceId);

    LARGE_INTEGER driverVersion;
    if (SUCCEEDED(adapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &driverVersion))) {
        uint64_t encoded = driverVersion.QuadPart;
        wprintf(
            L"Driver version: %d.%d.%d.%d\n", static_cast<uint16_t>((encoded >> 48) & 0xFFFF),
            static_cast<uint16_t>((encoded >> 32) & 0xFFFF),
            static_cast<uint16_t>((encoded >> 16) & 0xFFFF),
            static_cast<uint16_t>(encoded & 0xFFFF));
    }
    printf("\n");
}

int main(int argc, char** argv)
{
    uint32_t elemCount = 2;
    if (argc == 2)
      elemCount = atoi(argv[1]);

    elemCount = elemCount * 1000000;

    uint32_t trials = 1000;
    uint32_t dispatches = 1;
    uint32_t bufSizeInByte = elemCount * sizeof(uint16_t);
    uint32_t threadGroupSizeX = 256;

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

    ComPtr<IDXGIAdapter1> hardwareAdapter;
    ComPtr<ID3D12Device> device;
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND !=
        factory->EnumAdapters1(adapterIndex, &hardwareAdapter); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        hardwareAdapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        HRESULT hr = D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
        if (hr == S_OK)
        {
            PrintAdapterInfo(hardwareAdapter.Get());
            break;
        }
    }

    D3D12_DESCRIPTOR_RANGE1 rootDescriptorRanges[3] = {};
    {
        D3D12_DESCRIPTOR_RANGE1 rootDescriptorRange = {};
        rootDescriptorRange.BaseShaderRegister = 0;
        rootDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        rootDescriptorRange.RegisterSpace = 0;
        rootDescriptorRange.NumDescriptors = 1;
        rootDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        rootDescriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
        rootDescriptorRanges[0] = rootDescriptorRange;
    }
    {
        D3D12_DESCRIPTOR_RANGE1 rootDescriptorRange = {};
        rootDescriptorRange.BaseShaderRegister = 1;
        rootDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        rootDescriptorRange.RegisterSpace = 0;
        rootDescriptorRange.NumDescriptors = 1;
        rootDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        rootDescriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
        rootDescriptorRanges[1] = rootDescriptorRange;
    }
    {
        D3D12_DESCRIPTOR_RANGE1 rootDescriptorRange = {};
        rootDescriptorRange.BaseShaderRegister = 0;
        rootDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        rootDescriptorRange.RegisterSpace = 0;
        rootDescriptorRange.NumDescriptors = 1;
        rootDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        rootDescriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
        rootDescriptorRanges[2] = rootDescriptorRange;
    }
    D3D12_ROOT_PARAMETER1 rootParameter = {};
    rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter.DescriptorTable.NumDescriptorRanges = 3;
    rootParameter.DescriptorTable.pDescriptorRanges = rootDescriptorRanges;

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDescriptor = {};
    versionedRootSignatureDescriptor.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    versionedRootSignatureDescriptor.Desc_1_1.NumParameters = 1;
    versionedRootSignatureDescriptor.Desc_1_1.pParameters = &rootParameter;
    versionedRootSignatureDescriptor.Desc_1_1.NumStaticSamplers = 0;
    versionedRootSignatureDescriptor.Desc_1_1.pStaticSamplers = nullptr;
    versionedRootSignatureDescriptor.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signatureBlob;
    ComPtr<ID3DBlob> errors;
    ThrowIfFailed(D3D12SerializeVersionedRootSignature(&versionedRootSignatureDescriptor, &signatureBlob, &errors));

    ComPtr<ID3D12RootSignature> rootSignature;
    ThrowIfFailed(device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

    std::vector<char> csBytes;
    std::array<const char*, 3> possiblePaths = { "concat.cso", "..\\x64\\Debug\\concat.cso", "..\\x64\\Release\\concat.cso" };
    for (const char* path : possiblePaths) {
        std::ifstream csFile(path, std::fstream::binary);
        if (csFile.fail()) {
            continue;
        }
        csBytes = std::vector<char>((std::istreambuf_iterator<char>(csFile)), std::istreambuf_iterator<char>());
        csFile.close();
        break;
    }

    if (csBytes.empty()) {
        std::cout << "Failed to open concat.cso!" << std::endl;
        return -1;
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc = {};
    computePipelineStateDesc.CS.pShaderBytecode = csBytes.data();
    computePipelineStateDesc.CS.BytecodeLength = csBytes.size();
    computePipelineStateDesc.pRootSignature = rootSignature.Get();

    ComPtr<ID3D12PipelineState> computePipelineState;
    ThrowIfFailed(device->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&computePipelineState)));
    std::cout << "Creating compute pipeline state object succeeds!" << std::endl;

    ComPtr<ID3D12Resource> uniformBuffer;
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(256, D3D12_RESOURCE_FLAG_NONE),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&uniformBuffer))
    );

    ComPtr<ID3D12Resource> srcTensor;
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufSizeInByte, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, 0),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&srcTensor)));

    ComPtr<ID3D12Resource> dstTensor;
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufSizeInByte, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, 0),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&dstTensor)));

    ComPtr<ID3D12DescriptorHeap> cbSrvUavHeap;
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.NumDescriptors = 5;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbSrvUavHeap)));

    D3D12_UNORDERED_ACCESS_VIEW_DESC srcUavDesc = {};
    srcUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    srcUavDesc.Format = DXGI_FORMAT_UNKNOWN;
    srcUavDesc.Buffer.FirstElement = 0;
    srcUavDesc.Buffer.NumElements = elemCount;
    srcUavDesc.Buffer.StructureByteStride = (UINT)sizeof(uint16_t);
    srcUavDesc.Buffer.CounterOffsetInBytes = 0;
    srcUavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
    CD3DX12_CPU_DESCRIPTOR_HANDLE srcTensorUavHandle(cbSrvUavHeap->GetCPUDescriptorHandleForHeapStart());
    device->CreateUnorderedAccessView(srcTensor.Get(), nullptr, &srcUavDesc, srcTensorUavHandle);

    D3D12_UNORDERED_ACCESS_VIEW_DESC dstUavDesc = {};
    dstUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    dstUavDesc.Format = DXGI_FORMAT_UNKNOWN;
    dstUavDesc.Buffer.FirstElement = 0;
    dstUavDesc.Buffer.NumElements = elemCount;
    dstUavDesc.Buffer.StructureByteStride = (UINT)sizeof(uint16_t);
    dstUavDesc.Buffer.CounterOffsetInBytes = 0;
    dstUavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

    uint32_t cbvSrvUavHandleOffset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dstTensorUavHandle(srcTensorUavHandle.Offset(cbvSrvUavHandleOffset));
    device->CreateUnorderedAccessView(dstTensor.Get(), nullptr, &dstUavDesc, dstTensorUavHandle);


    CD3DX12_CPU_DESCRIPTOR_HANDLE uniformBufferHandle(dstTensorUavHandle.Offset(cbvSrvUavHandleOffset));
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = uniformBuffer->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (sizeof(Constants) + 255) & ~255; // CB size is required to be 256-byte aligned.;
        device->CreateConstantBufferView(&cbvDesc, uniformBufferHandle);
    }

    ComPtr<ID3D12QueryHeap> timestampQueryHeap;
    D3D12_QUERY_HEAP_DESC timestampHeapDesc = {};
    timestampHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    timestampHeapDesc.Count = 2 * trials;
    ThrowIfFailed(device->CreateQueryHeap(&timestampHeapDesc, IID_PPV_ARGS(&timestampQueryHeap)));
    ComPtr<ID3D12Resource> resolveBuffer;
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(2 * trials * sizeof(uint64_t), D3D12_RESOURCE_FLAG_NONE),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&resolveBuffer)));

    D3D12_COMMAND_QUEUE_DESC queueDesc = { D3D12_COMMAND_LIST_TYPE_DIRECT, 0, D3D12_COMMAND_QUEUE_FLAG_NONE };
    ComPtr<ID3D12CommandQueue> queue;
    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue)));
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));

    ComPtr<ID3D12GraphicsCommandList> commandList;
    ThrowIfFailed(device->CreateCommandList(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), computePipelineState.Get(),
        IID_PPV_ARGS(&commandList)));

    ID3D12DescriptorHeap* pHeaps[] = { cbSrvUavHeap.Get() };
    commandList->SetDescriptorHeaps(1, pHeaps);

    ComPtr<ID3D12Resource> uploadBuffer;
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(256),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)));

    ComPtr<ID3D12Resource> uploadBuffer2;
    ThrowIfFailed(device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
      D3D12_HEAP_FLAG_NONE,
      &CD3DX12_RESOURCE_DESC::Buffer(bufSizeInByte),
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&uploadBuffer2)));

    ComPtr<ID3D12Resource> uploadBuffer3;
    ThrowIfFailed(device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
      D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
      &CD3DX12_RESOURCE_DESC::Buffer(bufSizeInByte),
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS(&uploadBuffer3)));

    void* mapped = nullptr;
    uploadBuffer->Map(0, nullptr, &mapped);
    Constants* uniformData = static_cast<Constants*>(mapped);
    uniformData->StartIndex = 0;
    uniformData->Stride = 1;
    uniformData->EndIndex = elemCount;
    uploadBuffer->Unmap(0, nullptr);

    uint16_t* inputData;
    uploadBuffer2->Map(0, nullptr, reinterpret_cast<void**>(&inputData));
    for (uint32_t i = 0; i < elemCount; ++i)
    {
      
      if (i == elemCount - 1)
        inputData[i] = 2;
      else
        inputData[i] = i;
    }
    uploadBuffer2->Unmap(0, nullptr);

    commandList->CopyResource(srcTensor.Get(), uploadBuffer2.Get());

    commandList->CopyResource(uniformBuffer.Get(), uploadBuffer.Get());

    ResourceBarrier(commandList.Get(), uniformBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuSrvDescriptorHandle(cbSrvUavHeap->GetGPUDescriptorHandleForHeapStart());

    D3D12_RESOURCE_BARRIER barrierDesc = {};

    barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrierDesc.UAV.pResource = dstTensor.Get();

    for (uint32_t i = 0; i < trials; ++i) {
        commandList->SetComputeRootSignature(rootSignature.Get());
        commandList->SetComputeRootDescriptorTable(0, gpuSrvDescriptorHandle);
        commandList->SetPipelineState(computePipelineState.Get());

        commandList->EndQuery(timestampQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, i * 2);
        for (uint32_t d = 0; d < dispatches; ++d) {
            commandList->Dispatch((elemCount + threadGroupSizeX - 1)/threadGroupSizeX, 1, 1);
            commandList->ResourceBarrier(1, &barrierDesc);
        }
        commandList->EndQuery(timestampQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, i * 2 + 1);
    }

    commandList->CopyResource(uploadBuffer3.Get(), dstTensor.Get());

    ResourceBarrier(commandList.Get(), uploadBuffer3.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);

    commandList->ResolveQueryData(
        timestampQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, 2 * trials,
        resolveBuffer.Get(), 0);;

    ThrowIfFailed(commandList->Close());

    ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
    queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    ComPtr<ID3D12Fence> computeFence;
    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&computeFence)));
    UINT computeFenceValue = 1;

    // Create an event handle to use for frame synchronization.
    HANDLE computeFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (computeFenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }

    ThrowIfFailed(queue->Signal(computeFence.Get(), computeFenceValue));

    // Wait until the fence has been processed.
    ThrowIfFailed(computeFence->SetEventOnCompletion(computeFenceValue, computeFenceEvent));
    WaitForSingleObjectEx(computeFenceEvent, INFINITE, FALSE);

    uint64_t timestampFrequency;
    ThrowIfFailed(queue->GetTimestampFrequency(&timestampFrequency));

    uint16_t* inputData2;
    uploadBuffer3->Map(0, nullptr, reinterpret_cast<void**>(&inputData2));
    for (uint32_t i = 0; i < 100; ++i)
    {
      printf("data[%d] = %d\n", i, inputData2[i]);
    }
    printf("data[%d] = %d\n", elemCount - 1, inputData2[elemCount - 1]);
    uploadBuffer3->Unmap(0, nullptr);

    void* resolvePtr;
    resolveBuffer->Map(0, nullptr, &resolvePtr);

    const uint64_t* timestamps = static_cast<const uint64_t*>(resolvePtr);
    double min = std::numeric_limits<double>::infinity();
    double max = -1;
    double avg = 0;
    for (uint32_t i = 0; i < trials; ++i) {
        double duration = (timestamps[2 * i + 1] - timestamps[2 * i]) * 1000.0 / timestampFrequency;
        if (duration < min) {
            min = duration;
        }
        if (duration > max) {
            max = duration;
        }
        avg += duration / trials;
    }

    std::cout << "Min: " << min << " ms " << std::endl;
    std::cout << "Max: " << max << " ms " << std::endl;
    std::cout << "Avg: " << avg << " ms " << std::endl;


    return 0;
}

