struct VertexOutput
{
    float4 position : SV_Position;
    float3 normal;
    float2 uv       : TEXCOORD0;
};

struct Vertex {
  float3 position;
  float pad0;
  float3 normal;
  float pad1;
  float2 uv;
  float2 pad2;

};

struct WorldData {
  float4x4 view;
  float4x4 model;
  float4x4 proj;
  float3 cameraPos;

};

struct PushConstants {
    Vertex* vertexPtr;
    float4x4* transformPtr;
    WorldData* worldDataPtr;
    uint transformIndex;
    uint vertexOffset;
    uint texIndex;
};

[[vk::push_constant]]
PushConstants pc;


[shader("vertex")]
VertexOutput vertexMain(uint vertexID : SV_VertexID)
{
    VertexOutput output;

    Vertex v = pc.vertexPtr[vertexID + pc.vertexOffset];

    output.position = mul(pc.worldDataPtr.proj, mul(pc.worldDataPtr.view, mul(pc.transformPtr[pc.transformIndex], float4(v.position, 1.0))));
    output.normal    = v.normal;
    output.uv = v.uv;
    
    return output;
}

[vk::binding(0, 0)]
Sampler2D globalTextures[];

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
  input.normal *= 0.5;
  input.normal += 0.5;
  return float4(input.normal, 1.0);
}
