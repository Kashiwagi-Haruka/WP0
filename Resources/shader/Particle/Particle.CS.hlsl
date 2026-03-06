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

    if (gEmitter.emit == 2)
    {
        if (particleIndex < kMaxParticles)
        {
            gParticles[particleIndex] = (Particle) 0;
            gFreeList[particleIndex] = particleIndex;
        }

        if (particleIndex == 0)
        {
            gFreeListIndex[0] = int(kMaxParticles) - 1;
        }
        return;
    }

    if (particleIndex != 0)
    {
        return;
    }

    if (gEmitter.emit == 0)
    {
        return;
    }

    RandomGenerator generator;
    generator.seed = (float(particleIndex) + gPerFrame.time).xxx * max(gPerFrame.time, 0.0001f);

    for (uint countIndex = 0; countIndex < gEmitter.count; ++countIndex)
    {
        int freeListIndex;
        InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
        if (0 <= freeListIndex && freeListIndex < int(kMaxParticles))
        {
            int emitIndex = gFreeList[freeListIndex];

            const float kPi = 3.14159265f;
            float spreadFactor = saturate((gEmitter.emissionAngle - kPi) / kPi);
            float randomLength = generator.Generate1d() * gEmitter.radius;
            float randomAngle = generator.Generate1d() * 2.0f * kPi;
            float2 radialDir = float2(cos(randomAngle), sin(randomAngle));

            // emissionAngle が PI のときは直線、2PI のときは円形に広がる
            float2 planarDirection = lerp(float2(1.0f, 0.0f), radialDir, spreadFactor);
            if (length(planarDirection) < 0.0001f)
            {
                planarDirection = float2(1.0f, 0.0f);
            }
            planarDirection = normalize(planarDirection);
            float3 randomDirection = float3(planarDirection.x, planarDirection.y, 0.0f);

            gParticles[emitIndex].scale = gEmitter.particleScale;
            gParticles[emitIndex].translate = gEmitter.translate + randomDirection * randomLength;
            gParticles[emitIndex].velocity = randomDirection;
            gParticles[emitIndex].lifeTime = gEmitter.lifeTime;
            gParticles[emitIndex].currentTime = 0.0f;
            gParticles[emitIndex].beforeColor = gEmitter.beforeColor;
            gParticles[emitIndex].afterColor = gEmitter.afterColor;
        }
        else
        {
            InterlockedAdd(gFreeListIndex[0], 1);
            break;
        }
    }
}