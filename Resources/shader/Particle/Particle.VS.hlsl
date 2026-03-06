#include "Particle.hlsli"

StructuredBuffer<Particle> gParticles : register(t1);
ConstantBuffer<PerView> gPerView : register(b1);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;

    Particle particle = gParticles[instanceId];

    float4x4 worldMatrix = gPerView.billboardMatrix;
    worldMatrix[0] *= particle.scale.x;
    worldMatrix[1] *= particle.scale.y;
    worldMatrix[2] *= particle.scale.z;
    worldMatrix[3].xyz = particle.translate;

    float4x4 worldViewProjection = mul(worldMatrix, gPerView.viewProjection);

    output.position = mul(input.position, worldViewProjection);
    output.texcoord = input.texcoord;
    float t = (particle.lifeTime > 0.0f) ? saturate(particle.currentTime / particle.lifeTime) : 1.0f;
    output.color = lerp(particle.beforeColor, particle.afterColor, t);

    return output;
}