#include "LightHelper.fx"

#define BLOCK_SIZE 16
#define THREAD_PER_TILE (BLOCK_SIZE * BLOCK_SIZE)


struct SurfaceData
{
	float3 posVS;         // View space position
	float3 posWS;       // Screen space derivatives
	float3 normal;               // View space normal
	float4 diffuse;
	float specIntensity;        // Treated as a multiplier on albedo
	float specPow;

};
SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};



float4 TexToProj(float2 texCoord, float depth)
{
	float4 PosCS = float4(texCoord.x * 2.0f - 1.0f, (1.0f - texCoord.y) * 2.0f - 1.0f, depth, 1.0f);

	return PosCS;
}

float3 ProjToView(float4 csPos, float4x4 invProj)
{
	// Proj의 역행렬과 곱해줘서 View Space 공간 좌표 구해옴.
	float4 viewPos = mul(csPos, invProj);

	// 정규화 시켜서, 좌표 0~1 값으로
	viewPos.xyz = viewPos.xyz / viewPos.w;

	return viewPos.xyz;
}



float4 ComputLight(float2 texCoord, float3 posWS, SurfaceData surface, sPointLight light, float3 toEyeW, sDirectLight directLight, float ssaoFactor, float ShadowFactor, float ptShadowFactor)
{
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 ShadowColor = float3(1.0f, 1.0f, 1.0f);

	float distanceToEye = length(toEyeW);

	//struct SurfaceData
	//{
	//	float3 posVS;         // View space position
	//	float3 posWS;       // Screen space derivatives
	//	float3 normal;               // View space normal
	//	float4 diffuse;
	//	float specIntensity;        // Treated as a multiplier on albedo
	//	float specPow;

	//};
	float4 A, D, S;

	//void ComputePointLightDeferred(
	//	sPointLight L,
	//	float3 pos,
	//	float4 diffuseColor,
	//	float3 normal,
	//	float3 toEye,
	//	float specPow,
	//	float specInt,
	//	out float4 ambient,
	//	out float4 diffuse,
	//	out float4 spec
	//)

	//void ComputeDirectionLightDeferred(
	//	sDirectLight L,
	//	float4 diffuseColor,
	//	float3 normal,
	//	float3 toEye,
	//	float specPow,
	//	float specInt,
	//	out float4 ambient,
	//	out float4 diffuse,
	//	out float4 spec
	//)



	ComputePointLightDeferred(
			light,
			surface.posWS,
			float4(surface.diffuse.xyz, 1.0f),
			surface.normal,
			toEyeW,
			surface.specPow,
			surface.specIntensity,
			A, D, S);

		ambient += A * ssaoFactor ;
		diffuse += D * ssaoFactor * ShadowFactor;
		spec += S * ShadowFactor;

	float4 resultLight = (ambient + diffuse) + spec;
	resultLight.w = 1.0f;

	return resultLight;

	

}


