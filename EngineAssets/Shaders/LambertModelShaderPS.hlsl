#include "Common.hlsli"
#include "LambertFunctions.hlsli"

PixelOutput main(ModelVertexToPixel input)
{
	PixelOutput result;

	float2 scaledUV = input.texCoord0;
	
    float4 albedo = albedoTexture.Sample(defaultSampler, scaledUV).rgba;

	if (albedo.a <= alphaTestThreshold)
	{
		discard;
		result.color = float4(0.f, 0.f, 0.f, 0.f);
		return result;
	}

	float3 pointLights = 0; // <- The sum of all point lights.
	for(unsigned int p = 0; p < NumberOfLights; p++)
	{
#if 0
        pointLights += EvaluatePointLightLambert(
			albedo.rgb, 
			float3(0.0f, 0.0f, -1.0f),
			myPointLights[p].Color.rgb, 
			myPointLights[p].Range, 
			myPointLights[p].Position.xyz,
			input.worldPosition.xyz
		);
#else
        pointLights += EvaluateSoftAreaLightLambert(
			albedo.rgb, 
			float3(0.0f, 0.0f, -1.0f),
			myPointLights[p].Color.rgb, 
			myPointLights[p].Radius, 
			myPointLights[p].Range, 
			myPointLights[p].Position.xyz,
			input.worldPosition.xyz
		);
#endif
    }
	
    result.color.rgb = albedo.rgb + pointLights;
	result.color.a = albedo.a;
	return result;
}

