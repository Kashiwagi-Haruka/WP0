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
struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};
struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    float2 padding;
};
struct PointLightCount
{
    uint count;
    float3 padding;
};
struct SpotLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
    float cosFalloffStart;
    float2 padding;
};
struct SpotLightCount
{
    uint count;
    float3 padding;
};
struct AreaLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 normal;
    float width;
    float height;
    float radius;
    float decay;
    float padding;
};
struct AreaLightCount
{
    uint count;
    float3 padding;
};
struct Camera
{
    float3 worldPosition;
    float padding;
    float2 screenSize;
    int fullscreenGrayscaleEnabled;
    int fullscreenSepiaEnabled;
    float2 padding2;
};
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b3);
ConstantBuffer<Camera> gCamera : register(b4);
ConstantBuffer<PointLightCount> gPointLightCount : register(b5);
ConstantBuffer<SpotLightCount> gSpotLightCount : register(b6);
ConstantBuffer<AreaLightCount> gAreaLightCount : register(b7);
StructuredBuffer<SpotLight> gSpotLights : register(t2);
StructuredBuffer<PointLight> gPointLights : register(t1);
StructuredBuffer<AreaLight> gAreaLights : register(t3);
Texture2D<float4> gTexture : register(t0);
Texture2D<float4> gEnvironmentTexture : register(t4);
Texture2D<float> gShadowMap : register(t5);
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
float ComputeMicroShadow(float3 normal, float3 toLight, float3 toEye)
{
    // シャドウマップ以外の陰りはハーフランバートで制御する。
    // ※toEyeはインターフェース維持のため受け取る。
    float NdotL = saturate(dot(normal, toLight));
    (void) toEye;
    return pow(saturate(NdotL * 0.5f + 0.5f), 2.0f);
}

float ComputeShadowVisibility(float4 shadowPosition)
{
    if (shadowPosition.w <= 0.0f)
    {
        return 1.0f;
    }

    float3 shadowCoord = shadowPosition.xyz / shadowPosition.w;
    float2 shadowUV;
    shadowUV.x = shadowCoord.x * 0.5f + 0.5f;
    shadowUV.y = -shadowCoord.y * 0.5f + 0.5f;

    if (shadowUV.x < 0.0f || shadowUV.x > 1.0f || shadowUV.y < 0.0f || shadowUV.y > 1.0f)
    {
        return 1.0f;
    }

    float receiverDepth = shadowCoord.z;
    if (receiverDepth <= 0.0f || receiverDepth >= 1.0f)
    {
        return 1.0f;
    }

    uint shadowMapWidth;
    uint shadowMapHeight;
    gShadowMap.GetDimensions(shadowMapWidth, shadowMapHeight);
    float2 texelSize = 1.0f / float2(shadowMapWidth, shadowMapHeight);
    const float depthBias = 0.002f;

    float visibility = 0.0f;
    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            float2 sampleUV = saturate(shadowUV + float2(x, y) * texelSize);
            float shadowDepth = gShadowMap.Sample(gSampler, sampleUV);
            visibility += ((receiverDepth - depthBias) <= shadowDepth) ? 1.0f : 0.0f;
        }
    }

    visibility /= 9.0f;
    return lerp(0.25f, 1.0f, visibility);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    const float pi = 3.14159265f;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float2 distortionCenter = float2(0.5f, 0.5f);
    float2 offsetFromCenter = transformedUV.xy - distortionCenter;
    float distanceFromCenter = length(offsetFromCenter);
    float distortionWeight = saturate(distanceFromCenter * max(gMaterial.distortionFalloff, 0.001f));
    float twistAngle = distortionWeight * gMaterial.distortionStrength * (2.0f * pi);
    float sineValue = sin(twistAngle);
    float cosineValue = cos(twistAngle);
    float2 twistedOffset = float2(
        offsetFromCenter.x * cosineValue - offsetFromCenter.y * sineValue,
        offsetFromCenter.x * sineValue + offsetFromCenter.y * cosineValue);
    float radialScale = 1.0f + distortionWeight * abs(gMaterial.distortionStrength) * 0.2f;
    transformedUV.xy = distortionCenter + twistedOffset * radialScale;
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    if (gMaterial.enableLighting != 0)
    {
        ////half lambert
        float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        float3 directionalLightVector = -normalize(gDirectionalLight.direction);
        float NdotL = dot(normalize(input.normal), directionalLightVector);
        float cos = pow(saturate(NdotL * 0.5f + 0.5f), 2.0f);
        float3 halfVector = normalize(directionalLightVector + toEye);
        float NDotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NDotH), gMaterial.shininess);
        float directionalShadow = ComputeMicroShadow(normalize(input.normal), directionalLightVector, toEye);
        float shadowVisibility = ComputeShadowVisibility(input.shadowPosition);
        
        float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity * directionalShadow * shadowVisibility;
        float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f) * directionalShadow * shadowVisibility;
        
// Point Light
        float3 N = normalize(input.normal);
        
        float3 diffuseP = float3(0.0f, 0.0f, 0.0f);
        float3 specularP = float3(0.0f, 0.0f, 0.0f);
        for (uint index = 0; index < gPointLightCount.count; ++index)
        {
            PointLight pointLight = gPointLights[index];
            float3 Lp = normalize(pointLight.position - input.worldPosition);
            float3 H = normalize(Lp + toEye); // ハーフベクトル
            float distance = length(pointLight.position - input.worldPosition);
            float attenuation = pow(saturate(1.0f - distance / pointLight.radius), pointLight.decay);

// 拡散 (Lambert)
            float NdotL_p = saturate(dot(N, Lp));
            float pointShadow = ComputeMicroShadow(N, Lp, toEye);
            diffuseP += gMaterial.color.rgb * textureColor.rgb *
                  pointLight.color.rgb * pointLight.intensity *
                  NdotL_p * attenuation * pointShadow;

// 鏡面反射 (Phong)
            float NdotH_p = saturate(dot(N, H));
            float specularPowP = pow(NdotH_p, gMaterial.shininess);
            specularP += pointLight.color.rgb * pointLight.intensity *
                   specularPowP * attenuation * pointShadow;
        }

// Spot Light
        float3 spotLightDiffuse = float3(0.0f, 0.0f, 0.0f);
        float3 spotLightSpecular = float3(0.0f, 0.0f, 0.0f);
        for (uint spotIndex = 0; spotIndex < gSpotLightCount.count; ++spotIndex)
        {
            SpotLight spotLight = gSpotLights[spotIndex];
            float3 lightToSurface = spotLight.position - input.worldPosition;
            float3 lightDirection = normalize(lightToSurface);
            float3 spotDirection = normalize(spotLight.direction);
            float cosAngle = dot(lightDirection, spotDirection);
            float falloffFactor = saturate((cosAngle - spotLight.cosAngle) / (spotLight.cosFalloffStart - spotLight.cosAngle));
            float distanceToLight = length(lightToSurface);
            float attenuationFactor = pow(saturate(1.0f - distanceToLight / spotLight.distance), spotLight.decay);

            float NdotL_s = saturate(dot(N, lightDirection));
            float spotShadow = ComputeMicroShadow(N, lightDirection, toEye);
            float3 Hs = normalize(lightDirection + toEye);
            float NdotH_s = saturate(dot(N, Hs));
            float specularPowS = pow(NdotH_s, gMaterial.shininess);

            spotLightDiffuse += gMaterial.color.rgb * textureColor.rgb *
                spotLight.color.rgb * spotLight.intensity *
                NdotL_s * attenuationFactor * falloffFactor * spotShadow;
            spotLightSpecular += spotLight.color.rgb * spotLight.intensity *
                specularPowS * attenuationFactor * falloffFactor * spotShadow;
        }
        // Area Light
        float3 areaLightDiffuse = float3(0.0f, 0.0f, 0.0f);
        float3 areaLightSpecular = float3(0.0f, 0.0f, 0.0f);
        for (uint areaIndex = 0; areaIndex < gAreaLightCount.count; ++areaIndex)
        {
            AreaLight areaLight = gAreaLights[areaIndex];
            float3 lightToSurface = areaLight.position - input.worldPosition;
            float3 lightDirection = normalize(lightToSurface);
            float distanceToLight = length(lightToSurface);
            float attenuationFactor = pow(saturate(1.0f - distanceToLight / areaLight.radius), areaLight.decay);
            float lightFacing = saturate(dot(normalize(areaLight.normal), -lightDirection));
            float areaScale = areaLight.width * areaLight.height;

            float NdotL_a = saturate(dot(N, lightDirection));
            float areaShadow = ComputeMicroShadow(N, lightDirection, toEye);
            float3 Ha = normalize(lightDirection + toEye);
            float NdotH_a = saturate(dot(N, Ha));
            float specularPowA = pow(NdotH_a, gMaterial.shininess);
            float intensity = areaLight.intensity * areaScale * lightFacing;

            areaLightDiffuse += gMaterial.color.rgb * textureColor.rgb *
                areaLight.color.rgb * intensity *
                NdotL_a * attenuationFactor * areaShadow;
            areaLightSpecular += areaLight.color.rgb * intensity *
                specularPowA * attenuationFactor * areaShadow;
        }
        float3 viewDirection = normalize(input.worldPosition - gCamera.worldPosition);
        float3 reflectedDirection = reflect(viewDirection, normalize(input.normal));
        float2 environmentUV = float2(atan2(reflectedDirection.z, reflectedDirection.x) / (2.0f * pi) + 0.5f,
            asin(reflectedDirection.y) / pi + 0.5f);
        float3 environmentColor = gEnvironmentTexture.Sample(gSampler, environmentUV).rgb;

        output.color.rgb = diffuse + specular + diffuseP + specularP + spotLightDiffuse + spotLightSpecular + areaLightDiffuse + areaLightSpecular;
        output.color.rgb += environmentColor * gMaterial.environmentCoefficient;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    output.color.rgb = ApplyGrayscale(output.color.rgb);
    output.color.rgb = ApplySepia(output.color.rgb);
    if (textureColor.a < 0.1f)
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