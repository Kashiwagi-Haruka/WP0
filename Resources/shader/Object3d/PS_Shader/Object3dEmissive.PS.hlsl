#include "../Object3d.hlsli"
struct Material
{
    float4 color;
    int enableLighting;
    float3 padding;
    float4x4 uvTransform;
    float shininess;
    float environmentCoefficient;
    int grayscaleEnabled;
    int sepiaEnabled;
    float distortionStrength;
    float distortionFalloff;
};
ConstantBuffer<Material> gMaterial : register(b0);
struct Camera
{
    float3 worldPosition;
    float padding;
    float2 screenSize;
    int fullscreenGrayscaleEnabled;
    int fullscreenSepiaEnabled;
    float2 padding2;
};
ConstantBuffer<Camera> gCamera : register(b4);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};
float3 ApplyGrayscale(float3 color)
{
    if (gMaterial.grayscaleEnabled == 0 && gCamera.fullscreenGrayscaleEnabled == 0)
    {
        return color;
    }
    float y = dot(color, float3(0.2125f, 0.7154f, 0.0721f));
    return float3(y, y, y);
}
float3 ApplySepia(float3 color)
{
    if (gMaterial.sepiaEnabled == 0 && gCamera.fullscreenSepiaEnabled == 0)
    {
        return color;
    }

    float3 sepia;
    sepia.r = dot(color, float3(0.393f, 0.769f, 0.189f));
    sepia.g = dot(color, float3(0.349f, 0.686f, 0.168f));
    sepia.b = dot(color, float3(0.272f, 0.534f, 0.131f));
    return saturate(sepia);
}
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);

    float3 baseColor = textureColor.rgb * gMaterial.color.rgb;
    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    float3 normal = normalize(input.normal);
    float rim = 1.0f - saturate(dot(normal, toEye));
    float softRim = smoothstep(0.0f, 0.9f, rim);
    float glow = pow(softRim, 1.4f);
    float halo = pow(softRim, 3.0f);
    float luminance = dot(baseColor, float3(0.2126f, 0.7152f, 0.0722f));
    float glowMask = smoothstep(0.2f, 0.85f, luminance);
    float glowRamp = smoothstep(0.35f, 0.95f, softRim);
    float glowStrength = 1.0f + saturate(gMaterial.environmentCoefficient) * 2.0f;
    float3 glowColor = baseColor * glowStrength * (glowMask * 1.2f + glowRamp * 1.6f);

    output.color.rgb = baseColor * (1.4f + glow * 3.0f) + baseColor * (halo * 2.5f) + glowColor;
    output.color.a = textureColor.a * gMaterial.color.a;
    output.color.rgb = ApplyGrayscale(output.color.rgb);
    output.color.rgb = ApplySepia(output.color.rgb);
    if (textureColor.a < 0.5f)
    {
        discard;
    }
    if (textureColor.a == 0.0f)
    {
        discard;
    }
    if (output.color.a == 0.0f)
    {
        discard;
    }

    return output;
}