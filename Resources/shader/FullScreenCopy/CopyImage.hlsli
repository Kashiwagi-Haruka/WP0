struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

cbuffer PostEffectParameters : register(b0)
{
    float vignetteStrength;
    float randomNoiseEnabled;
    float randomNoiseScale;
    float randomNoiseTime;
    float randomNoiseBlendMode;
};