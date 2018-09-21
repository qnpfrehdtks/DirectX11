


#define BLOCK_SIZE 16
#define MAX_REFLECTION_RAY_MARCH_STEP 0.02f
#define NUM_RAY_MARCH_SAMPLES 16

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
	AddressU = BORDER;
	AddressV = BORDER;

};

cbuffer perFrame
{
	//float g_FarZ;
	float g_ReflectPower;
	float3 g_EyePosWS;
	float4x4 g_TextureToWorld;
	float4x4 g_ViewProj;
	float4x4 g_View;
	float4x4 g_InvProj;
	float4x4 g_InvView;
	float4x4 g_InvViewProj;
	float4x4 g_Proj;

};

Texture2D g_NormalMap;
Texture2D g_DepthMap;
Texture2D g_DiffuseMap;
Texture2D g_SpecMap;

// 텍스처 출력 변수.
RWTexture2D<float4> g_OutputMap;

#define NUM_RAY_MARCH_SAMPLES 16


// 위아래 Z 값 차이 없음.
float LinearDepthFromDepth(float depth)
{
	return g_Proj._43 / (depth - g_Proj._33);
}

float4 VStoSS(float3 posV)
{
	float4 PosCS = mul(float4(posV,1.0f), g_Proj);
	PosCS.xyz /= PosCS.w;

	float4 PosSS = float4(0.5f * PosCS.x + 0.5f, -0.5f * PosCS.y + 0.5f, PosCS.z, 1.0f);

	return PosSS;
}

float4 TexToVS(float2 Tex, float depth)
{
	  float4 PosCS = float4( 2.0f * Tex.x - 1.0f, -2.0f * Tex.y + 1.0f, depth, 1.0f);
	  float4 PosVS = mul(PosCS, g_InvProj);
	  PosVS /= PosVS.w;


	  return PosVS;
}

float4 TexToWS(float2 Tex, float depth)
{
	float4 PosCS = float4(2.0f * Tex.x - 1.0f, -2.0f * Tex.y + 1.0f, depth, 1.0f);
	float4 PosWS = mul(PosCS, g_InvViewProj);
	PosWS /= PosWS.w;

	PosWS.w = 1.0f;
	//float3 PosWS = mul(float4(PosVS.xyz,1.0f), g_InvView).xyz;

	return PosWS;
}


bool GetReflection(
	float3 ScreenSpaceReflectionVec,
	float3 ScreenSpacePos,
	out float3 ReflectionColor)
{

	float3 PreRaySample = ScreenSpacePos;

	// Raymarch in the direction of the ScreenSpaceReflectionVec until you get an intersection with your z buffer
	for (int RayStepIdx = 0; RayStepIdx<NUM_RAY_MARCH_SAMPLES; RayStepIdx++)
	{
		float3 RaySample = ((RayStepIdx + 1) * MAX_REFLECTION_RAY_MARCH_STEP) * ScreenSpaceReflectionVec + ScreenSpacePos;
		float ZBufferVal = g_DepthMap.SampleLevel(samPoint, RaySample.xy, 0).r;

		float linearZBufferVal = LinearDepthFromDepth(ZBufferVal);
		float linearSampleVal = LinearDepthFromDepth(RaySample.z);

		if (linearSampleVal > linearZBufferVal)
		{
			float3 MaxSamplePos = RaySample;
			float3 MinSamplePos = PreRaySample;
			float3 MidSamplePos;

			for (int BinaryIdx = 0; BinaryIdx < 12; BinaryIdx++)
			{
				MidSamplePos = lerp(MinSamplePos, MaxSamplePos, 0.5f);
				float ZMidBufferVal = g_DepthMap.SampleLevel(samPoint, MidSamplePos.xy, 0).r;

				float linearMidZBufferVal = LinearDepthFromDepth(ZMidBufferVal);
				float linearMidSampleVal = LinearDepthFromDepth(MidSamplePos.z);

				if (linearMidSampleVal > linearMidZBufferVal)
					MaxSamplePos = MidSamplePos;
				else
					MinSamplePos = MidSamplePos;

			}

			float2 UVSamplingAttenuation = smoothstep(0.05, 0.1, MidSamplePos.xy) * (1 - smoothstep(0.95, 1, MidSamplePos.xy));
			UVSamplingAttenuation.x *= UVSamplingAttenuation.y;

			if (UVSamplingAttenuation.x > 0)
			{
				ReflectionColor = g_DiffuseMap.SampleLevel(samPoint, MidSamplePos.xy, 0).rgb;
				return true;
			}
			else
			{
				return false;
			}
		}

		PreRaySample = RaySample;
	}

	return false;
}



[numthreads(16, 16, 1)]
void main(
	uint3 groupId : SV_GroupID,
	int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID,
	uint groupIndex : SV_GroupIndex,
	uniform bool isSSAO)
{
	float2 globalCoord = dispatchThreadID.xy;
	float2 texCoord = globalCoord.xy * rcp(float2(g_DiffuseMap.Length.x, g_DiffuseMap.Length.y));
	float4 diffuseColor = g_DiffuseMap.SampleLevel(samPoint, texCoord, 0.0f);
	float isSSR = g_SpecMap.SampleLevel(samPoint, texCoord, 0.0f).y;

	if (isSSR < 0.8f)
	{
		g_OutputMap[globalCoord] = diffuseColor;
		return;
	}

	float3 normalW = g_NormalMap.SampleLevel(samPoint, texCoord, 0.0f) * 2.0f - 1.0f;
	float3 normalV = normalize(mul(normalW, (float3x3)g_View));
	
	float depth = g_DepthMap.SampleLevel(samPoint, texCoord, 0.0f).x;
	float linearDepth = LinearDepthFromDepth(depth);

	float4 PosV = TexToVS(texCoord, depth);
	float3 ViewDirV = PosV.xyz;

	float3 RefelctedRayV = reflect(ViewDirV, normalV);

	/*float CameraFacingReflectionAttenuation = 1 - smoothstep(0.25, 0.5, dot(-ViewDirV, RefelctedRayV));

	[branch]
	if (CameraFacingReflectionAttenuation <= 0)
	{
		g_OutputMap[globalCoord] = diffuseColor;
		return;
	}*/

	float4 PosS = VStoSS(PosV);
	float4 RefelctedPosV = (g_ReflectPower * float4(RefelctedRayV,0.0f) ) + PosV;
	//RefelctedPosV.w = 1.0f;
	float4 RefelctedPosS = VStoSS(RefelctedPosV);

	float2 reflectedRayS = normalize(RefelctedPosS.xy - PosS.xy);

	g_OutputMap[globalCoord] =  g_DiffuseMap.SampleLevel(samPoint, PosS.xy, 0.0f);
	float3 OutColor = 0;
	if (GetReflection(float3(reflectedRayS, 0.0f), RefelctedPosS, OutColor))
	{
		g_OutputMap[globalCoord] = float4(OutColor,1.0f);
	}
	else
	{
		g_OutputMap[globalCoord] = diffuseColor;
	}




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

