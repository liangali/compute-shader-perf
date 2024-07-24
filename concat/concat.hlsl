//RWTexture2D<float4> dst_tensor_image2d : register(u0);
RWStructuredBuffer<int> dstBuffer : register(u0); // Unordered Access View (UAV)

// Texture2D<float4> src_tensor_image2d : register(t1);
StructuredBuffer<int> srcBuffer: register(t1);      // Shader Resource View (SRV)

cbuffer cbuffer_U : register(b4) {
  uint4 U[2];
};
groupshared float4 weights_cache[32];

struct tint_symbol_1 {
  uint3 lid : SV_GroupThreadID;
  uint3 gid : SV_DispatchThreadID;
  uint3 wid : SV_GroupID;
};

void main_inner(uint3 gid, uint3 wid, uint3 lid) {
    const int index = gid.x;
    dstBuffer[index] = srcBuffer[index] + index;
}

[numthreads(256, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.gid, tint_symbol.wid, tint_symbol.lid);
  return;
}