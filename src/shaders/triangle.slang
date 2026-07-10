struct VertexOutput
{
    float4 position : SV_Position;
    float3 color    : COLOR;
};

struct Vertex {
  float3 position;
  float pad0;
  float3 normal;
  float pad1;
  float2 uv;
  float2 pad2;

};

struct PushConstants {
    Vertex* vertexPtr;
};

[[vk::push_constant]]
PushConstants pc;


[shader("vertex")]
VertexOutput vertexMain(uint vertexID : SV_VertexID)
{
    VertexOutput output;

    Vertex v = pc.vertexPtr[vertexID];
    
    output.position = float4(v.position, 1.0);
    output.color    = v.normal;
    
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}
