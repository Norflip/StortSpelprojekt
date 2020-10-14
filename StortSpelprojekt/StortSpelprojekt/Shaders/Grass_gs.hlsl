#include "CommonBuffers.hlsl"
#include "IO.hlsl"

/*Alter the commented values to change the grass*/
[maxvertexcount(4)]
void main(line DS_OUTPUT_GRASS input[2], inout TriangleStream<GS_OUTPUT_GRASS> TriangleOutputStream)
{
	GS_OUTPUT_GRASS output = (GS_OUTPUT_GRASS)0;

	float bladeHeight = input[0].bladeHeight;

	float3 pos0World = input[0].position.xyz;
	float3 pos1World = input[1].position.xyz;

	float3 right = input[0].expandVector;

	pos0World.xyz += input[0].displacement;
	pos1World.xyz += input[1].displacement;

	float3 grassDirection = normalize(pos1World.xyz - pos0World.xyz);
	grassDirection = grassDirection;

	
	float grassBladeScale = 1.5f;//grassWidth
	float powfactor = 0.5; //pow factor
	float maxHeightPow = pow(bladeHeight, powfactor);
	float origWidth1 = grassBladeScale * (maxHeightPow - pow(input[0].height, powfactor));
	float origWidth2 = grassBladeScale * (maxHeightPow - pow(input[1].height, powfactor));

	
	float w = mul(float4(pos1World, 1.0f), mvp).w;
	float pixelRadius = w * pixelSize;

	
	float width1 = max(pixelRadius, origWidth1);
	float width2 = max(pixelRadius, origWidth2);

	float3 pos = pos0World.xyz - right * width1 / 2;
	output.posToEye = mul(float4(pos, 1), world);

	output.position = mul(float4(pos, 1.0f), mvp);
	output.tex = float2(0, input[0].tex.y);
	output.normal = input[0].normal;
	output.colour = input[0].colour;
	output.grassDirection = grassDirection;
	output.fade = origWidth1 / width1;

	TriangleOutputStream.Append(output);

	pos = pos1World.xyz - right * width2 / 2;
	output.posToEye = mul(float4(pos, 1), world);

	output.position = mul(float4(pos, 1.0f), mvp);
	output.tex = float2(0, input[1].tex.y);
	output.normal = input[1].normal;
	output.colour = input[1].colour;
	output.grassDirection = grassDirection;
	output.fade = origWidth2 / width2;

	TriangleOutputStream.Append(output);

	pos = pos0World.xyz + right * width1 / 2;
	output.posToEye = mul(float4(pos, 1), world);

	output.position = mul(float4(pos, 1.0f), mvp);
	output.tex = float2(1, input[0].tex.y);
	output.normal = input[0].normal;
	output.colour = input[0].colour;
	output.grassDirection = grassDirection;
	output.fade = origWidth1 / width1;

	TriangleOutputStream.Append(output);

	pos = pos1World.xyz + right * width2 / 2;

	output.posToEye = mul(float4(pos, 1), world);

	output.position = mul(float4(pos, 1.0f), mvp);
	output.tex = float2(1, input[1].tex.y);
	output.normal = input[1].normal;
	output.colour = input[1].colour;
	output.grassDirection = grassDirection;
	output.fade = origWidth2 / width2;

	TriangleOutputStream.Append(output);
}