#include "../Object3d.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 LightWVP;
    float4x4 World;
    float4x4 WorldInverseTranspose;
};

struct TextureCamera
{
    float4x4 textureViewProjection;
    float4x4 portalCameraWorld;
    float4x4 textureWorldViewProjection;
    float3 textureWorldPosition;
    int usePortalProjection;
    float3 padding;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct PortalVertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPosition : POSITION0;
    float4 shadowPosition : TEXCOORD1;
    float4 textureProjectedPosition : TEXCOORD2;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b1);
ConstantBuffer<TextureCamera> gTextureCamera : register(b5);

PortalVertexShaderOutput main(VertexShaderInput input)
{
    PortalVertexShaderOutput output;

    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix.WorldInverseTranspose));
    output.texcoord = input.texcoord;
    output.worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
    output.shadowPosition = mul(input.position, gTransformationMatrix.LightWVP);
    output.textureProjectedPosition = mul(input.position, mul(gTransformationMatrix.World, gTextureCamera.textureViewProjection));
    return output;
}
