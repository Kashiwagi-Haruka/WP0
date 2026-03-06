#include "CopyImage.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float rand2dTo1d(float2 value)
{
    const float2 smallValue = sin(value);
    const float2 dotDir = float2(12.9898f, 78.233f);
    float random = dot(smallValue, dotDir);
    return frac(sin(random) * 143758.5453f);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);

    float2 centeredUv = input.texcoord * (1.0f - input.texcoord.yx);
    float vignette = centeredUv.x * centeredUv.y * 16.0f;
    vignette = saturate(pow(vignette, 0.8f));
    float vignetteFactor = lerp(1.0f, vignette, saturate(vignetteStrength));
    output.color.rgb *= vignetteFactor;

    if (randomNoiseEnabled > 0.5f)
    {
        float random = rand2dTo1d(input.texcoord * randomNoiseScale * randomNoiseTime);
        float3 noiseColor = float3(random, random, random);

        if (randomNoiseBlendMode < 0.5f)
        {
            output.color.rgb = noiseColor;
        }
        else if (randomNoiseBlendMode < 1.5f)
        {
            output.color.rgb += noiseColor;
        }
        else if (randomNoiseBlendMode < 2.5f)
        {
            output.color.rgb -= noiseColor;
        }
        else if (randomNoiseBlendMode < 3.5f)
        {
            output.color.rgb *= noiseColor;
        }
        else
        {
            output.color.rgb = 1.0f - ((1.0f - output.color.rgb) * (1.0f - noiseColor));
        }

        output.color.rgb = saturate(output.color.rgb);
    }

    return output;
}