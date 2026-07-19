#include "common.hlsli"

float3 hash33(float3 p3)
{
    p3 = frac(p3 * float3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz + 33.33);
    return frac((p3.xxy + p3.yxx) * p3.zyx);
}

float noise(float3 x)
{
    // grid
    float3 i = floor(x);
    float3 f = frac(x);
    
    // quintic interpolant
    float3 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);
    
    // gradients
    float3 ga = hash33(i + float3(0.0, 0.0, 0.0));
    float3 gb = hash33(i + float3(1.0, 0.0, 0.0));
    float3 gc = hash33(i + float3(0.0, 1.0, 0.0));
    float3 gd = hash33(i + float3(1.0, 1.0, 0.0));
    float3 ge = hash33(i + float3(0.0, 0.0, 1.0));
    float3 gf = hash33(i + float3(1.0, 0.0, 1.0));
    float3 gg = hash33(i + float3(0.0, 1.0, 1.0));
    float3 gh = hash33(i + float3(1.0, 1.0, 1.0));
    
    // projections
    float va = dot(ga, f - float3(0.0, 0.0, 0.0));
    float vb = dot(gb, f - float3(1.0, 0.0, 0.0));
    float vc = dot(gc, f - float3(0.0, 1.0, 0.0));
    float vd = dot(gd, f - float3(1.0, 1.0, 0.0));
    float ve = dot(ge, f - float3(0.0, 0.0, 1.0));
    float vf = dot(gf, f - float3(1.0, 0.0, 1.0));
    float vg = dot(gg, f - float3(0.0, 1.0, 1.0));
    float vh = dot(gh, f - float3(1.0, 1.0, 1.0));
	
    // interpolation
    return va +
           u.x * (vb - va) +
           u.y * (vc - va) +
           u.z * (ve - va) +
           u.x * u.y * (va - vb - vc + vd) +
           u.y * u.z * (va - vc - ve + vg) +
           u.z * u.x * (va - vb - ve + vf) +
           u.x * u.y * u.z * (-va + vb + vc - vd + ve - vf - vg + vh);
}

ModelVertexToPixel main(ModelVertexInput input)
{
    ModelVertexToPixel result;

    float3 objectPosition = ObjectToWorld._m30_m31_m32;
    
    float4 vertexObjectPos = input.position;
    float4 vertexWorldPos = mul(ObjectToWorld, vertexObjectPos);
    
    // xz-cylinder mask
    float radius = 0.19f;
    float radialIntensity = saturate(0.01f * length(vertexObjectPos.xz - objectPosition.xz) - radius);
    
    // height mask
    float height = 0.14f;
    float heightIntensity = saturate(0.01f * (vertexObjectPos.y - objectPosition.y) - height);
    
    float intensity = radialIntensity * heightIntensity;
    
    vertexWorldPos.y += intensity * 65.0f * noise(vertexWorldPos.xyz * 0.008f + float3(1.2f * TotalTime, 0.0f, 0.0f));
    float4 vertexViewPos = mul(WorldToCamera, vertexWorldPos);
    float4 vertexProjectionPos = mul(CameraToProjection, vertexViewPos);

    float3x3 toWorldRotation = (float3x3) ObjectToWorld;
    float3 vertexWorldNormal = mul(toWorldRotation, input.normal);
    float3 vertexWorldBinormal = mul(toWorldRotation, input.binormal);
    float3 vertexWorldTangent = mul(toWorldRotation, input.tangent);

    result.position = vertexProjectionPos;
    result.worldPosition = vertexWorldPos;
    result.vertexColor0 = input.vertexColor0;
    result.vertexColor1 = input.vertexColor1;
    result.vertexColor2 = input.vertexColor2;
    result.vertexColor3 = input.vertexColor3;

    result.texCoord0 = input.texCoord0;
    result.texCoord1 = input.texCoord1;
    result.texCoord2 = input.texCoord2;
    result.texCoord3 = input.texCoord3;

    result.normal = vertexWorldNormal;
    result.binormal = vertexWorldBinormal;
    result.tangent = vertexWorldTangent;

    return result;
}