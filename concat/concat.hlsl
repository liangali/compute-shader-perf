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
  float4 r_w0_h0_s0 = (0.0f).xxxx;
  float4 r_w0_h0_s1 = (0.0f).xxxx;
  float4 r_w0_h0_s2 = (0.0f).xxxx;
  float4 r_w0_h0_s3 = (0.0f).xxxx;
  uint filters_offset = uint(((DST_S * 4) * asint(U[0].z)));
  int s = 0;
  while(true) {
    GroupMemoryBarrierWithGroupSync();
    if ((lid.x < 32u)) {
      weights_cache[lid.x] = asfloat(weights_buffer.Load4((16u * (filters_offset + lid.x))));
    }
    GroupMemoryBarrierWithGroupSync();
    float4 src_w0_h0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    src_w0_h0 = src_tensor_image2d.Load(int3(DST_X, ((DST_Y * asint(U[0].z)) + s), 0));
    s = (s + 1);
    r_w0_h0_s0 = (r_w0_h0_s0 + (weights_cache[0] * src_w0_h0.x));
    r_w0_h0_s0 = (r_w0_h0_s0 + (weights_cache[1] * src_w0_h0.y));
    r_w0_h0_s0 = (r_w0_h0_s0 + (weights_cache[2] * src_w0_h0.z));
    r_w0_h0_s0 = (r_w0_h0_s0 + (weights_cache[3] * src_w0_h0.w));
    r_w0_h0_s1 = (r_w0_h0_s1 + (weights_cache[4] * src_w0_h0.x));
    r_w0_h0_s1 = (r_w0_h0_s1 + (weights_cache[5] * src_w0_h0.y));
    r_w0_h0_s1 = (r_w0_h0_s1 + (weights_cache[6] * src_w0_h0.z));
    r_w0_h0_s1 = (r_w0_h0_s1 + (weights_cache[7] * src_w0_h0.w));
    r_w0_h0_s2 = (r_w0_h0_s2 + (weights_cache[8] * src_w0_h0.x));
    r_w0_h0_s2 = (r_w0_h0_s2 + (weights_cache[9] * src_w0_h0.y));
    r_w0_h0_s2 = (r_w0_h0_s2 + (weights_cache[10] * src_w0_h0.z));
    r_w0_h0_s2 = (r_w0_h0_s2 + (weights_cache[11] * src_w0_h0.w));
    r_w0_h0_s3 = (r_w0_h0_s3 + (weights_cache[12] * src_w0_h0.x));
    r_w0_h0_s3 = (r_w0_h0_s3 + (weights_cache[13] * src_w0_h0.y));
    r_w0_h0_s3 = (r_w0_h0_s3 + (weights_cache[14] * src_w0_h0.z));
    r_w0_h0_s3 = (r_w0_h0_s3 + (weights_cache[15] * src_w0_h0.w));
    src_w0_h0 = src_tensor_image2d.Load(int3(DST_X, ((DST_Y * asint(U[0].z)) + s), 0));
    r_w0_h0_s0 = (r_w0_h0_s0 + (weights_cache[16] * src_w0_h0.x));
    r_w0_h0_s0 = (r_w0_h0_s0 + (weights_cache[17] * src_w0_h0.y));
    r_w0_h0_s0 = (r_w0_h0_s0 + (weights_cache[18] * src_w0_h0.z));
    r_w0_h0_s0 = (r_w0_h0_s0 + (weights_cache[19] * src_w0_h0.w));
    r_w0_h0_s1 = (r_w0_h0_s1 + (weights_cache[20] * src_w0_h0.x));
    r_w0_h0_s1 = (r_w0_h0_s1 + (weights_cache[21] * src_w0_h0.y));
    r_w0_h0_s1 = (r_w0_h0_s1 + (weights_cache[22] * src_w0_h0.z));
    r_w0_h0_s1 = (r_w0_h0_s1 + (weights_cache[23] * src_w0_h0.w));
    r_w0_h0_s2 = (r_w0_h0_s2 + (weights_cache[24] * src_w0_h0.x));
    r_w0_h0_s2 = (r_w0_h0_s2 + (weights_cache[25] * src_w0_h0.y));
    r_w0_h0_s2 = (r_w0_h0_s2 + (weights_cache[26] * src_w0_h0.z));
    r_w0_h0_s2 = (r_w0_h0_s2 + (weights_cache[27] * src_w0_h0.w));
    r_w0_h0_s3 = (r_w0_h0_s3 + (weights_cache[28] * src_w0_h0.x));
    r_w0_h0_s3 = (r_w0_h0_s3 + (weights_cache[29] * src_w0_h0.y));
    r_w0_h0_s3 = (r_w0_h0_s3 + (weights_cache[30] * src_w0_h0.z));
    r_w0_h0_s3 = (r_w0_h0_s3 + (weights_cache[31] * src_w0_h0.w));
    s = (s + 1);
    filters_offset = (filters_offset + 32u);
    if ((s >= asint(U[0].z))) {
      break;
    }
  }
  bool tint_tmp = (DST_Y >= asint(U[0].x));
  if (!tint_tmp) {
    tint_tmp = (DST_S >= asint(U[0].y));
  }
  if ((tint_tmp)) {
    return;
  }
  if (((DST_S + 0) >= asint(U[0].y))) {
    return;
  }
  {
    float4 bias_val = asfloat(biases_buffer.Load4((16u * uint((DST_S + 0)))));
    {
      float4 res = (r_w0_h0_s0 + bias_val);
      dst_tensor_image2d[int2((DST_X + 0), (((DST_Y + 0) * asint(U[0].y)) + (DST_S + 0)))] = res;
    }
  }
  if (((DST_S + 1) >= asint(U[0].y))) {
    return;
  }
  {
    float4 bias_val = asfloat(biases_buffer.Load4((16u * uint((DST_S + 1)))));
    {
      float4 res = (r_w0_h0_s1 + bias_val);
      dst_tensor_image2d[int2((DST_X + 0), (((DST_Y + 0) * asint(U[0].y)) + (DST_S + 1)))] = res;
    }
  }
  if (((DST_S + 2) >= asint(U[0].y))) {
    return;
  }
  {
    float4 bias_val = asfloat(biases_buffer.Load4((16u * uint((DST_S + 2)))));
    {
      float4 res = (r_w0_h0_s2 + bias_val);
      dst_tensor_image2d[int2((DST_X + 0), (((DST_Y + 0) * asint(U[0].y)) + (DST_S + 2)))] = res;
    }
  }
  if (((DST_S + 3) >= asint(U[0].y))) {
    return;
  }
  {
    float4 bias_val = asfloat(biases_buffer.Load4((16u * uint((DST_S + 3)))));
    {
      float4 res = (r_w0_h0_s3 + bias_val);
      dst_tensor_image2d[int2((DST_X + 0), (((DST_Y + 0) * asint(U[0].y)) + (DST_S + 3)))] = res;
    }
  }
}

[numthreads(64, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.gid, tint_symbol.wid, tint_symbol.lid);
  return;
}