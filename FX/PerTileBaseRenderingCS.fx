// 
// Source�� ������ DirectX11 å ����.

#include "PerTileBaseLightingCS.fx"


// =========================================
//                  Texture
// =========================================

#define MAX_REFLECTION_RAY_MARCH_STEP 0.03f
#define NUM_RAY_MARCH_SAMPLES 16

cbuffer LightCount
{
	uint g_lightCount;
};

cbuffer PerFrame
{
	float4x4 g_InvProj;
	float4x4 g_ViewProj;
	float4x4 g_WorldViewProj;
	float4 g_PerspectiveValues;
	//float4x4 g_InvViewProj;
	float4x4 g_InvView;
	float4x4 g_View;
	float4x4 g_Proj;

	float4 g_EyePosW;
	
	bool g_isNormal;
	bool g_isSSAO;
	bool g_isTileBase;
	bool g_isDepth;
	bool g_isDiffuse;

	

};

// ===========================================
//                 SamplerState
// ===========================================

SamplerComparisonState samShadow
{
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	ComparisonFunc = LESS;
};


SamplerState samTriLinearSam
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};


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
	// ���, Ȯ�뿡�� ����, �Ӹ��� �� ���͸�.
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

// ===========================================
//              Map & CS Variables;
// ===========================================

Texture2D g_NormalMap;
Texture2D g_SpecPowMap;
Texture2D g_SSAOMap;
TextureCube g_ShadowCubeMap;
//Texture2D g_ShadowMap;
Texture2D g_DepthMap;
Texture2D g_DiffuseSpecMap;

// �ؽ�ó ��� ����.
RWTexture2D<float4> g_OutputMap;

StructuredBuffer<sPointLight> g_LightList;
StructuredBuffer<sDirectLight> g_DirectLight;

groupshared uint g_minDepthInt;
groupshared uint g_maxDepthInt;

groupshared uint g_visibleLightCount = 0;
groupshared uint g_visibleLightIdx[801];

static const float PixelSize = 2.0 / 768.0f;
// Frustum Intersect �˻�. 
bool intersects(sPointLight light, in float4 frustumPlanes[6])
{
	bool intersectsFrustum = true;

	// ���� ��ġ View Space�� ��ȯ
	float4 LightPosV = mul(float4(light.pos, 1.0f), g_View);

	// Frustum�� 6���� ����� ���ư��� ���� �����ϴ��� �Ǻ��Ѵ�.
	[unroll]
	for (uint i = 0; i < 6; i++)
	{
		// ������ �Ÿ��� ���ϱ� ���� ���� ��ġ�� ����� �����Ѵ�.
		float dist = dot(frustumPlanes[i], LightPosV);

		if (dist <= light.range)
		{
			// ���� �����Ÿ� �ȿ� ��ٸ� �����Ѵٰ� ����
			intersectsFrustum = true;
		}
		else
			return false;
	}

	return intersectsFrustum;
}

// �ش� Ÿ���� �ּ� ����, �ִ� ���̰��� ������, Frustum ����.
void buildFrustum(in uint2 groupId, in float farZ, in float nearZ, float width, float height, out float4 frustumPlanes[6])
{

	uint TileXNum =  width * rcp(BLOCK_SIZE);
	uint TileYNum = height * rcp(BLOCK_SIZE);

	// Ÿ���� �ϳ��� View Frustum����
	const uint XMinID = BLOCK_SIZE * groupId.x;
	const uint YMinID = BLOCK_SIZE * groupId.y;

	const uint XMaxID = BLOCK_SIZE * (groupId.x + 1);
	const uint YMaxID = BLOCK_SIZE * (groupId.y + 1);

	// Clip Space�� ��ȯ [ 0 ~ 1]
	float left = (float)XMinID * rcp(width);
	float right = (float)XMaxID * rcp(width);
	float top = (float)( YMinID)* rcp(height);
	float bottom = (float)( YMaxID)* rcp(height);


	left = min(left, 1.0f);
	right = min(right, 1.0f);
	top = min(top, 1.0f);
	bottom = min(bottom, 1.0f);


	// �ð� �����
	// 0----------1
	// |          |
	// |          |
	// |          |
	// |          |
	// 3----------2
	float4 FrustumCornerCS[4];
	FrustumCornerCS[0] = float4(left * 2.0f - 1.0f, (1.0f - top) * 2.0f - 1.0f, 1.0f, 1.0f);
	FrustumCornerCS[1] = float4(right * 2.0f - 1.0f, (1.0f - top) * 2.0f - 1.0f, 1.0f, 1.0f);
	FrustumCornerCS[2] = float4(right * 2.0f - 1.0f, (1.0f - bottom) * 2.0f - 1.0f, 1.0f, 1.0f);
	FrustumCornerCS[3] = float4(left * 2.0f - 1.0f, (1.0f - bottom) * 2.0f - 1.0f, 1.0f, 1.0f);

	float4 FrustumCornerVS[6];
	FrustumCornerVS[0] = mul(FrustumCornerCS[0], g_InvProj);
	FrustumCornerVS[1] = mul(FrustumCornerCS[1], g_InvProj);
	FrustumCornerVS[2] = mul(FrustumCornerCS[2], g_InvProj);
	FrustumCornerVS[3] = mul(FrustumCornerCS[3], g_InvProj);

	FrustumCornerVS[0] * rcp( FrustumCornerVS[0].w);
	FrustumCornerVS[1] * rcp(FrustumCornerVS[1].w);
	FrustumCornerVS[2] * rcp(FrustumCornerVS[2].w);
	FrustumCornerVS[3] * rcp(FrustumCornerVS[3].w);


	[unroll]
	for (uint i = 0; i < 4; i++) {
		float4 n;
		n.xyz = cross(FrustumCornerVS[i], FrustumCornerVS[(i + 1) & 3]).xyz;
		n.w = 0.0f;
		n = -normalize(n);

		frustumPlanes[i] = n;
	}

	// Near/far clipping planes in view space
	frustumPlanes[4] = float4(0.0f, 0.0f, -1.0f, nearZ );
	frustumPlanes[5] = float4(0.0f, 0.0f, 1.0f, -farZ );

}




float4 VStoSS(float3 PosVS)
{
	float4 clipSpace = mul(float4(PosVS, 1.0f), g_Proj);
	float3 NDCSpace = clipSpace.xyz / clipSpace.w;
	float3 ScreenSpace = NDCSpace.xyz * float3(0.5f ,-0.5f, 0.5f) + float3(0.5f, 0.5f,0.5f);
	return float4(ScreenSpace, 1.0f);

}
//http://www.dice.se/wp-content/uploads/2014/12/GDC11_DX11inBF3_Public.pdf //
// ������. ����3 ������ GDC ���� �ڷ�
//
//float PointShadow(float3 ToPixel)
//{
//	float sampledDistance =  g_ShadowCubeMap.SampleLevel(samShadow, ToPixel, Depth);
//	float Distance = length(ToPixel);
//
//	if (Distance < sampledDistance + 0.0001f)
//		return 1.0f;
//	else return 0.5f;
//
//
//}

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void main(
	uint3 groupId : SV_GroupID,
	int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID,
	uint groupIndex : SV_GroupIndex,
	uniform bool isSSAO,
	uniform bool isDepth = false,
	uniform bool isNormal = false,
	uniform bool isDiffuse = false,
	uniform bool isSpec = false

     )
{
	// �� Ÿ���� 16X16�� Pixel�� �����Ǿ� �ִµ�.
	// �� Ÿ���� Pixel�鿡�� local idx ����.  0 1 2 3 4 5 ~ 15
	//                               16 17 18  
	uint2 globalCoord = dispatchThreadID.xy;
	float2 texCoord = globalCoord.xy * rcp( float2(g_DiffuseSpecMap.Length.x, g_DiffuseSpecMap.Length.y));

	float depth = g_DepthMap.SampleLevel(samPoint, texCoord, 0.0f).x;

	float4 posCS = TexToProj(texCoord, depth);
	float3 posVS = ProjToView(posCS, g_InvProj);
	float4 posWS = mul(float4(posVS, 1.0f), g_InvView);

	uint depthInt = asuint(posVS.z);

	// Ÿ���� ���̰�, �ִ밪�� �ּڰ��� ã��.
	// dipatchThreadID�� Global�� ��ǥ��.
	// 0 ~ 1024, 0 ~ 768 �� �ػ� ũ�⸸ŭ ����.
	g_minDepthInt = 0x7F7FFFFF;
	g_maxDepthInt = 0;

	GroupMemoryBarrierWithGroupSync();

	InterlockedMin(g_minDepthInt, depthInt);
	InterlockedMax(g_maxDepthInt, depthInt);

	GroupMemoryBarrierWithGroupSync();

	float minGroupDepth = asfloat(g_minDepthInt);
	float maxGroupDepth = asfloat(g_maxDepthInt);

	// ============= Frustum Building Calcul ==================
	//(in uint2 groupId, in float farZ, in float nearZ, float width, float height, out float4 frustumPlanes[6])
	float4 frustumPlanes[6];
	buildFrustum(groupId.xy, maxGroupDepth, minGroupDepth,1024, 768, frustumPlanes);

	//=============================================================

	// ========== Light Culling =============
	uint ThreadCount = THREAD_PER_TILE;
	uint passCount = (g_lightCount + ThreadCount - 1) * rcp( ThreadCount );

	// Thread�� �ϳ��� �׷쿡 100���ִٰ� �����ϰ�, ���� 1000���� �ִٰ� ����.
	// �׷�. �׷쿡 �ִ� thread 0�� 0�� �� 100�� �� 200�� ���� ���� ����ü �ø� ����� �ð� �ϰ�
	// �ش� ���� �� Ÿ�Ͽ� ��ģ�ٸ�, �ش� ���� �ε��� ��ȣ�� ����Ʈ�� ����ϰ�, ��ġ�� ������ ���� ���� ������ ����Ѵ�.
	// thread 1�� ���������� 1�� ��, 101�� ��, 201�� ���� culling �Ǵ��� üũ�Ѵ�.
	// �̰� �ݺ�.

	for (int i = 0; i < passCount; i++) {
		uint lightIndex = (i * ThreadCount) + groupIndex;

		lightIndex = min(lightIndex, g_lightCount);

		if (intersects(g_LightList[lightIndex], frustumPlanes)) {
			uint offset;
			InterlockedAdd(g_visibleLightCount, 1, offset); // �����Ѵٸ�, LightCount ����.
			g_visibleLightIdx[offset] = lightIndex; // Shared Memory�� Light�� Index �߰�
		}
	}

	// ��� Thread �� ���� ������ ��ġ�⸦ ��޸�.
	GroupMemoryBarrierWithGroupSync();

	// ============== Gbuffer���� ���� ������. =================== //

	SurfaceData surface;
	surface.posWS = posWS.xyz;
	//surface.ShadowPosH = mul(float4(vin.PosL, 1.0f), g_ShadowTransform);
	surface.normal = g_NormalMap[globalCoord].xyz * 2.0f - 1.0f;
	float4 diffuseSpec = g_DiffuseSpecMap[globalCoord];
	surface.diffuse.xyz = diffuseSpec.xyz;
	surface.specIntensity = diffuseSpec.w;

	float3 specSSRShadow = g_SpecPowMap[globalCoord].xyz;
	surface.specPow = specSSRShadow.x;


	// ============= Light Acculmurate Calcul ==================
	float4 compositedLighting = float4(0, 0, 0, 0);

	float3 toEyeW =  g_EyePosW - posWS.xyz;
	toEyeW = normalize(toEyeW);
//	float4 ambient = float4(0.10f, 0.10f, 0.05f, 0.0f);

	// =====================================================
	//                   SSAO Calcul
	// ================================================
	float ssaoFactor = 1.0f;

	if (isSSAO)
		ssaoFactor = g_SSAOMap.SampleLevel(samLinear, texCoord, 0.0f).x;

	// =====================================================
	//            Screen Space Reflectio Shader
	// =====================================================
	//(float3 PosVS, float3 PosCS, float LinDepth, float3 NormalVS, out float3 OutReflectionColor)

	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 A, D, S;

	ComputeDirectionLightDeferred(
		g_DirectLight[0],
		float4(surface.diffuse.xyz, 1.0f),
		surface.normal,
		toEyeW,
		surface.specPow,
		surface.specIntensity,
		A, D, S);

	ambient += A * ssaoFactor;
	diffuse += D * ssaoFactor *specSSRShadow.z;
	spec += S * specSSRShadow.z;

	//RayTest(float3 curPosWS, float curTexCoord, float3 PointLightPos, float curDepth, SamplerState samPoint, Texture2D DepthMap)

	float pointLightShadowFactor;
	for (int i = 0; i < g_visibleLightCount; i++)
	{
		uint lightIndex = g_visibleLightIdx[i];
		sPointLight light = g_LightList[lightIndex];

		//pointLightShadowFactor = PointShadowPCF( posWS - light.pos );
		//float3 shadowFactor = g_ShadowCubeMap.SampleLevel(samTriLinearSam, ,0.0f);

		compositedLighting += ComputLight(
			texCoord, 
			posWS, 
			surface, 
			light, 
			toEyeW, 
			g_DirectLight[0], 
			ssaoFactor,specSSRShadow.z, 0);
	}

	compositedLighting += ( ambient + diffuse + spec);


	float3 outColor = float3(0, 0, 0);
	float blendSpec = 1.0f;


	if (g_isNormal)
		g_OutputMap[globalCoord] = float4(surface.normal, 1.0f);
	else if (g_isSSAO)
		g_OutputMap[globalCoord] = ssaoFactor;
	else if (g_isTileBase)
		g_OutputMap[globalCoord] = (g_visibleLightCount/ 196.0f) * 2;
	else if (g_isDepth)
		g_OutputMap[globalCoord] = posVS.z / 2000.0f;
	else if (g_isDiffuse)
		g_OutputMap[globalCoord] = diffuseSpec;
	else
	g_OutputMap[globalCoord] = compositedLighting;

}



//ComputeDirectionLightDeferred(
//	g_DirectLight[0],
//	float4(surface.diffuse.xyz, 1.0f),
//	surface.normal,
//	toEyeW,
//	surface.specPow,
//	surface.specIntensity,
//	A, D, S);

//ambient += A;
//diffuse += D;
//spec += S;


technique11 TileTest
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, main(false)));
	}
}

technique11 TileSSAO
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, main(true)));
	}
}


// =======================================
//          Debug ��
// ======================================



technique11 Depth
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, main(true,true)));
	}
}

technique11 Normal
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, main(true,false, true)));
	}
}


technique11 Diffuse
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, main(true,false, false,true)));
	}
}

technique11 Spec
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, main(true,false, false, false ,true)));
	}
}




