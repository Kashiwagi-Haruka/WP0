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

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float4 GetGridLineColor(int lineIndex)
{
    if (lineIndex == 0)
    {
        return float4(1.0f, 1.0f, 0.0f, 1.0f); // origin: yellow
    }

    int oneDigit = abs(lineIndex) % 10;
    if (oneDigit == 0)
    {
        return float4(1.0f, 0.0f, 0.0f, 1.0f); // ones digit 0: red
    }
    if (oneDigit == 5)
    {
        return float4(0.0f, 1.0f, 0.0f, 1.0f); // ones digit 5: green
    }
    return float4(1.0f, 1.0f, 1.0f, 1.0f); // others: white
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    const float spacing = max(gMaterial.distortionFalloff, 0.001f);
    const float halfLineCount = max(gMaterial.distortionStrength, 1.0f);
    const float halfExtent = spacing * halfLineCount;
    const float lineHalfWidth = max(gMaterial.environmentCoefficient, 0.002f);

    float2 gridPos = input.worldPosition.xz;
    if (abs(gridPos.x) > halfExtent || abs(gridPos.y) > halfExtent)
    {
        clip(-1.0f);
    }

    float2 indexF = gridPos / spacing;
    int indexX = (int) round(indexF.x);
    int indexY = (int) round(indexF.y);

    float distX = abs(gridPos.x - ((float) indexX * spacing));
    float distY = abs(gridPos.y - ((float) indexY * spacing));

    float aaX = fwidth(gridPos.x) * 1.25f;
    float aaY = fwidth(gridPos.y) * 1.25f;
    float maskX = 1.0f - smoothstep(lineHalfWidth, lineHalfWidth + aaX, distX);
    float maskY = 1.0f - smoothstep(lineHalfWidth, lineHalfWidth + aaY, distY);
    float lineMask = max(maskX, maskY);
    clip(lineMask - 0.001f);

    float4 colorX = GetGridLineColor(indexX);
    float4 colorY = GetGridLineColor(indexY);
    float useY = step(maskX, maskY);
    float4 lineColor = lerp(colorX, colorY, useY);

    output.color = float4(lineColor.rgb * gMaterial.color.rgb, lineMask * gMaterial.color.a);
    return output;
}
