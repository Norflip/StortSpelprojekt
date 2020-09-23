#include "PhongShading.hlsl"

Texture2D diffuseMap : register (t0);
Texture2D normalMap : register (t1);

SamplerState defaultSampleType : register (s0);

struct VS_OUTPUT
{
	float4 position		 : SV_POSITION;
	float3 normal		 : NORMAL;
	float2 uv			 : TEXCOORD0;
	float3 worldPosition : POSITION;
	float3 tangent		 : TANGENT;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float4 textureColor = float4(0.5f, 0.5f, 0.5f, 1.0f);
	float4 normalmap = float4(1.0f, 1.0f, 1.0f, 1.0f);

	if (hasAlbedo)
		textureColor = diffuseMap.Sample(defaultSampleType, input.uv);

	if (hasNormalMap)
	{
		normalmap = normalMap.Sample(defaultSampleType, input.uv);
		input.normal = CalculateNormalMapping(input.normal, input.tangent, normalmap);
	}

	float3 normalized = normalize(input.normal);

	float3 viewDirection = cameraPosition - input.worldPosition;

	float4 finalColor = float4(0.0f, 0.0f, 0.0f, 0.0);

	for (int i = 0; i < nrOfPointLights; i++)
	{
		finalColor += CalculateLight(pointLights[i], normalized, input.worldPosition, viewDirection);
	}

	finalColor *= textureColor;

	//return float4(input.normal, 1.0f);
	return finalColor;
}
