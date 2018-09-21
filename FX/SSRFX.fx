#include "LightHelper.fx"

texture2D g_NormalMap;
texture2D g_DepthMap;
texture2D g_DiffuseMap;

// 텍스처 출력 변수.
RWTexture2D<float4> g_OutputMap;

#define BLOCK_SIZE 16
#define THREAD_PER_TILE (BLOCK_SIZE * BLOCK_SIZE)

// ===========================================
//                 SamplerState
// ===========================================

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;

};
SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	// Set a very far depth value if sampling outside of the NormalDepth map
	// so we do not get false occlusions.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 1e5f);
};

SamplerState samRandomVec
{
	// 축소, 확대에는 선형, 밉맵은 점 필터링.
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;

};


cbuffer perFrame
{
	float3 g_EyePosWS;
	float4x4 g_TextureToWorld;
	float4x4 g_ViewProj;
};


#define MAX_REFLECTION_RAY_MARCH_STEP 0.02f
#define NUM_RAY_MARCH_SAMPLES 16

bool RayCast(
	float3 ScreenSpaceReflectionVec,
	float3 ScreenSpacePos,
	out float3 ReflectionColor)
{
	// Raymarch in the direction of the ScreenSpaceReflectionVec until you get an intersection with your z buffer
	for (int RayStepIdx = 0; RayStepIdx<NUM_RAY_MARCH_SAMPLES; RayStepIdx++)
	{
		float3 RaySample = (RayStepIdx * MAX_REFLECTION_RAY_MARCH_STEP) * ScreenSpaceReflectionVec + ScreenSpacePos;
		float ZBufferVal = g_DepthMap.SampleLevel(samPoint, RaySample.xy, 0).r;

		if (RaySample.z > ZBufferVal)
		{
			ReflectionColor = g_DiffuseMap.SampleLevel(samPoint, RaySample.xy, 0).rgb;
			return true;
		}
	}

	return false;
}





[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void main(
	uint3 groupId : SV_GroupID,
	int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID,
	uint groupIndex : SV_GroupIndex,
	uniform bool isSSAO
)
{
	uint2 globalCoord = dispatchThreadID.xy;
	float2 texCoord = globalCoord.xy * rcp(float2(g_DiffuseMap.Length.x, g_DiffuseMap.Length.y));
	float2 NDCPos = texCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);

	float3 PosWS = mul(float4(NDCPos, 1.0f, 0.0f), g_TextureToWorld);
	float Depth = g_DepthMap.SampleLevel(samPoint, texCoord, 0.0f).x;
	float3 CamPos = normalize(PosWS - g_EyePosWS);
	float3 NormalWS = g_NormalMap.SampleLevel(samPoint, texCoord, 0.0f).xyz;

	float4 PosSS = float4(texCoord, Depth, 1.0f);

	float3 ReflectedDirWS = reflect(CamPos, NormalWS);

	// Compute second sreen space point so that we can get the SS reflection vector
	float4 ReflectionPosWS = float4(10.f*ReflectedDirWS + PosWS, 1.f);
	float4 ReflectionPosSS = mul(ReflectionPosWS, g_ViewProj);
	ReflectionPosSS /= ReflectionPosSS.w;
	ReflectionPosSS.xy = ReflectionPosSS.xy * float2(0.5, -0.5) + float2(0.5, 0.5);

	// Compute the sreen space reflection vector as the difference of the two screen space points
	float3 ReflectionDirSS = normalize(ReflectionPosSS.xyz - PosSS.xyz);

	float3 OutReflectionColor;
	RayCast(ReflectionDirSS, PosSS.xyz, OutReflectionColor);

	g_OutputMap[globalCoord] = float4( OutReflectionColor,1.0f);

}

technique11 SSRTest
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, main(false)));
	}
}

