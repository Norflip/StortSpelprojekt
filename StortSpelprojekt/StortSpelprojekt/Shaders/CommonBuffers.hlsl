
cbuffer cb_Object : register(b0)
{
	float4x4 mvp;
	float4x4 world;
};

cbuffer cb_Scene : register(b1)
{

}

cbuffer cb_Material : register(b2)
{

}

cbuffer SkeletonData : register(b3)
{
	float4x4 bones[60];
};