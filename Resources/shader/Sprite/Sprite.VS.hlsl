#include "Sprite.hlsli"

struct VertexInput
{
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
};

cbuffer Transform : register(b1)
{
    matrix WVP;
    matrix World;
};

VertexShaderOutput main(VertexInput input)
{
    VertexShaderOutput o;
    o.position = mul(input.position, WVP);
    o.texcoord = input.texcoord;
    o.normal = float3(0, 0, -1);
    return o;
}
