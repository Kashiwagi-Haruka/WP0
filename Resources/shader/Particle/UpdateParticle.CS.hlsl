#include "Particle.hlsli"

static const uint kMaxParticles = 4096;

RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);
ConstantBuffer<EmitterSphere> gEmitter : register(b0);
ConstantBuffer<PerFrame> gPerFrame : register(b1);

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint particleIndex = DTid.x;
    if (particleIndex >= kMaxParticles)
    {
        return;
    }

    Particle particle = gParticles[particleIndex];
    if (particle.beforeColor.a == 0.0f && particle.afterColor.a == 0.0f)
    {
        return;
    }

    particle.velocity += gEmitter.acceleration * gPerFrame.deltaTime;
    particle.translate += particle.velocity * gPerFrame.deltaTime;
    particle.currentTime += gPerFrame.deltaTime;

    if (particle.lifeTime <= 0.0f || particle.currentTime >= particle.lifeTime)
    {
        particle.beforeColor.a = 0.0f;
        particle.afterColor.a = 0.0f;
    }
    else
    {
        // フェードは描画時に currentTime / lifeTime から算出する
    }

    if (particle.beforeColor.a <= 0.0f && particle.afterColor.a <= 0.0f)
    {
        particle.scale = float3(0.0f, 0.0f, 0.0f);

        int freeListIndex;
        InterlockedAdd(gFreeListIndex[0], 1, freeListIndex);
        if ((freeListIndex + 1) < int(kMaxParticles))
        {
            gFreeList[freeListIndex + 1] = (uint) particleIndex;
        }
        else
        {
            InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
        }
    }

    gParticles[particleIndex] = particle;
}
