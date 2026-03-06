struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float4 color : COLOR0;
};

struct Particle
{
    float3 translate;
    float pad0;
    float3 scale;
    float lifeTime;
    float3 velocity;
    float currentTime;
    float4 beforeColor;
    float4 afterColor;
};

struct EmitterSphere
{
    float3 translate;
    float radius;
    uint count;
    float frequency;
    float frequencyTime;
    uint emit;
    float lifeTime;
    float3 acceleration;
    float pad0;
    float3 particleScale;
    float4 beforeColor;
    float4 afterColor;
    float emissionAngle;
    float3 emissionAnglePadding;
};

struct PerFrame
{
    float time;
    float deltaTime;
    float2 pad;
};

struct PerView
{
    float4x4 viewProjection;
    float4x4 billboardMatrix;
};

float rand3dTo1d(float3 value, float3 dotDir)
{
    float3 smallValue = sin(value);
    float random = dot(smallValue, dotDir);
    random = frac(sin(random) * 143758.5453f);
    return random;
}

float3 rand3dTo3d(float3 value)
{
    return float3(
        rand3dTo1d(value, float3(12.989f, 78.233f, 37.719f)),
        rand3dTo1d(value, float3(39.346f, 11.135f, 83.155f)),
        rand3dTo1d(value, float3(73.156f, 52.235f, 9.151f))
    );
}

struct RandomGenerator
{
    float3 seed;

    float3 Generate3d()
    {
        seed = rand3dTo3d(seed);
        return seed;
    }

    float Generate1d()
    {
        float result = rand3dTo1d(seed, float3(12.989f, 78.233f, 37.719f));
        seed.x = result;
        return result;
    }
};