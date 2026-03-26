#version 450

layout(push_constant) uniform DrawConstants {
    float screenWidth;
    float screenHeight;
    float alphaRef;
    uint flags;
} drawConstants;

layout(set = 0, binding = 0) uniform texture2D sceneTexture;
layout(set = 0, binding = 1) uniform sampler sceneSampler;

layout(location = 0) in vec2 inUv;
layout(location = 0) out vec4 outColor;

float ComputeFxaaLuma(vec3 rgb)
{
    return dot(rgb, vec3(0.299, 0.587, 0.114));
}

void main()
{
    vec2 invResolution = vec2(
        1.0 / max(drawConstants.screenWidth, 1.0),
        1.0 / max(drawConstants.screenHeight, 1.0));

    vec4 centerSample = texture(sampler2D(sceneTexture, sceneSampler), inUv);
    vec3 rgbM = centerSample.rgb;
    float lumaM = ComputeFxaaLuma(rgbM);

    vec3 rgbNW = texture(sampler2D(sceneTexture, sceneSampler), inUv + vec2(-1.0, -1.0) * invResolution).rgb;
    vec3 rgbNE = texture(sampler2D(sceneTexture, sceneSampler), inUv + vec2(1.0, -1.0) * invResolution).rgb;
    vec3 rgbSW = texture(sampler2D(sceneTexture, sceneSampler), inUv + vec2(-1.0, 1.0) * invResolution).rgb;
    vec3 rgbSE = texture(sampler2D(sceneTexture, sceneSampler), inUv + vec2(1.0, 1.0) * invResolution).rgb;

    float lumaNW = ComputeFxaaLuma(rgbNW);
    float lumaNE = ComputeFxaaLuma(rgbNE);
    float lumaSW = ComputeFxaaLuma(rgbSW);
    float lumaSE = ComputeFxaaLuma(rgbSE);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    float lumaRange = lumaMax - lumaMin;
    float threshold = max(0.03125, lumaMax * 0.125);
    if (lumaRange < threshold) {
        outColor = centerSample;
        return;
    }

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = (lumaNW + lumaSW) - (lumaNE + lumaSE);

    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 / 8.0),
        1.0 / 128.0);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, vec2(-8.0), vec2(8.0)) * invResolution;

    vec3 rgbA = 0.5 * (
        texture(sampler2D(sceneTexture, sceneSampler), inUv + dir * (1.0 / 3.0 - 0.5)).rgb +
        texture(sampler2D(sceneTexture, sceneSampler), inUv + dir * (2.0 / 3.0 - 0.5)).rgb);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(sampler2D(sceneTexture, sceneSampler), inUv + dir * -0.5).rgb +
        texture(sampler2D(sceneTexture, sceneSampler), inUv + dir * 0.5).rgb);
    float lumaB = ComputeFxaaLuma(rgbB);

    outColor = vec4((lumaB < lumaMin || lumaB > lumaMax) ? rgbA : rgbB, centerSample.a);
}