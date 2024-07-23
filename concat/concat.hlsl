RWTexture2D<float4> dst_tensor_image2d : register(u0);

Texture2D<float4> src_tensor_image2d : register(t1);
ByteAddressBuffer biases_buffer : register(t2);
ByteAddressBuffer weights_buffer : register(t3);

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
  int DST_X = int(gid.x) % asint(U[0].w);
  int DST_Y = (int(gid.x) / asint(U[0].w)) % asint(U[1].x);
  int DST_S = int(wid.y);
  DST_S = (DST_S * 4);
  if ((DST_S >= asint(U[0].y))) {
    return;
  }
  float4 s0 = (0.0f).xxxx;
  float4 s1 = (0.1f).xxxx;
  float4 s2 = (0.2f).xxxx;
  float4 s3 = (0.3f).xxxx;
  uint filters_offset = uint(((DST_S * 4) * asint(U[0].z)));
  int s = 0;

  dst_tensor_image2d[int2((DST_X + 0), (((DST_Y + 0) * asint(U[0].y)) + (DST_S + 0)))] = s0;
  dst_tensor_image2d[int2((DST_X + 0), (((DST_Y + 0) * asint(U[0].y)) + (DST_S + 1)))] = s1;
  dst_tensor_image2d[int2((DST_X + 0), (((DST_Y + 0) * asint(U[0].y)) + (DST_S + 2)))] = s2;
  dst_tensor_image2d[int2((DST_X + 0), (((DST_Y + 0) * asint(U[0].y)) + (DST_S + 3)))] = s3;
}

[numthreads(64, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.gid, tint_symbol.wid, tint_symbol.lid);
  return;
}