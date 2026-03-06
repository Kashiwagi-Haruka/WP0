#include "../Object3d.hlsli"
struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 LightWVP;
    float4x4 World;
    float4x4 WorldInverseTranspose;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b1);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    
};
VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix.WorldInverseTranspose));
    output.texcoord = input.texcoord;
    output.worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
    // PS側で常に参照されるため、未初期化にならないように必ず書き込む
    output.shadowPosition = mul(input.position, gTransformationMatrix.LightWVP);
    return output;
}