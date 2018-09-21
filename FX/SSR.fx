#include "LightHelper.fx"



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
	float4x4 g_WorldViewProj;
	float4x4 g_WorldView;
	float4x4 g_TextureToWorld;
	float4x4 g_ViewProj;
	float4x4 g_View;
};

struct VertexIn
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_Position;
	float4 PosV : TEXCOORD0;
	float3 NormalV : TEXCOORD1;
	float3 PosCS : TEXCOORD2;
	float2 Tex : TEXCOORD3;

};


//-----------------------------------------------------------------------------------------
// Pixel shader
//-----------------------------------------------------------------------------------------

cbuffer SSReflectionVSConstants 
{
	float4x4 g_Proj;
	float4x4 g_InvProj;
	float4x4 g_InvViewProj;
	float g_ViewAngleThreshold;
	float g_EdgeDistThreshold;
	float g_DepthBias;
	float g_ReflectionScale;
	float4 g_PerspectiveValues;
}


// 위아래 Z 값 차이 없음.
float LinearDepthFromDepth(float depth)
{
	return g_Proj._43 / (depth - g_Proj._33);
}

float4 VStoSS(float3 posV)
{
	float4 PosCS = mul(float4(posV, 1.0f), g_Proj);
	PosCS.xyz /= PosCS.w;

	float4 PosSS = float4(0.5f * PosCS.x + 0.5f, -0.5f * PosCS.y + 0.5f, PosCS.z, 1.0f);

	return PosSS;
}

float4 TexToVS(float2 Tex, float depth)
{
	float4 PosCS = float4(2.0f * Tex.x - 1.0f, -2.0f * Tex.y + 1.0f, depth, 1.0f);
	float4 PosVS = mul(PosCS, g_InvProj);
	PosVS /= PosVS.w;

	return PosVS;
}

float4 TexToCS(float2 Tex, float depth)
{
	float4 PosCS = float4(2.0f * Tex.x - 1.0f, -2.0f * Tex.y + 1.0f, depth, 1.0f);

	return PosCS;
}

float4 ViewToCS(float3 View)
{
	float4 PosCS = mul(float4(View, 1.0f), g_Proj);
	PosCS /= View.z;

	return PosCS;
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


float ConvertZToLinearDepth(float depth)
{
	float linearDepth = g_PerspectiveValues.z / (depth + g_PerspectiveValues.w);
	return linearDepth;
}

float3 CalcViewPos(float2 csPos, float depth)
{
	float3 position;

	position.xy = csPos.xy * g_PerspectiveValues.xy * depth;
	position.z = depth;

	return position;
}





// Pixel size in clip-space
// This is resulotion dependent
// Pick the minimum of the HDR width and height
static const float PixelSize = 2.0 / 768.0f;

// Number of sampling steps
// This is resulotion dependent
// Pick the maximum of the HDR width and height
static const int nNumSteps = 400;


Texture2D g_NormalMap;
Texture2D g_DepthMap;
Texture2D g_DiffuseMap;
Texture2D g_SpecMap;
// 텍스처 출력 변수.
RWTexture2D<float4> g_OutputMap;



VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(vin.PosL, g_WorldViewProj);
	vout.PosV = mul(vin.PosL, g_WorldView);
	vout.NormalV = mul(vin.NormalL, g_WorldView);
	vout.Tex = vin.Tex;

	vout.PosCS = vout.PosH.xyz / vout.PosH.w;

	return vout;

}






float4 PS(VertexOut pin) : SV_TARGET
{
	float3 PosVS = pin.PosV.xyz;
	float3 normalVS = normalize(pin.NormalV);
	float3 viewDirVS = normalize(pin.PosV.xyz);
	float3 reflectVS = reflect(viewDirVS, normalVS);
	float4 reflectColor = float4(0.0, 0.0, 0.0, 0.0);
	float2 tex = pin.Tex;


	float4 diffuseColor = g_DiffuseMap.SampleLevel(samPoint, tex, 0.0f);
	float isSSR = g_SpecMap.SampleLevel(samPoint, pin.Tex, 0.0f).y;

	if (isSSR < 0.9f)
	{
		return diffuseColor;
	}

	
	


	// Don't bother with reflected vector above the threshold vector
	if (reflectVS.z >= g_ViewAngleThreshold)
	{
		// Fade the reflection as the view angles gets close to the threshold
		float viewAngleThresholdInv = 1.0 - g_ViewAngleThreshold;
		float viewAngleFade = saturate(3.0 * (reflectVS.z - g_ViewAngleThreshold) / viewAngleThresholdInv);

		// Transform the View Space Reflection to clip-space
		float3 vsPosReflect = PosVS + reflectVS;
		float3 csPosReflect = mul(float4(vsPosReflect, 1.0), g_Proj).xyz / vsPosReflect.z;
		float3 csReflect = csPosReflect - pin.PosCS;

		// Resize Screen Space Reflection to an appropriate length.
		float reflectScale = PixelSize / length(csReflect.xy);
		csReflect *= reflectScale;

		// Calculate the first sampling position in screen-space
		float2 ssSampPos = (pin.PosCS + csReflect).xy;
		ssSampPos = ssSampPos * float2(0.5, -0.5) + 0.5;

		// Find each iteration step in screen-space
		float2 ssStep = csReflect.xy * float2(0.5, -0.5);

		// Build a plane laying on the reflection vector
		// Use the eye to pixel direction to build the tangent vector
		float4 rayPlane;
		float3 vRight = cross(viewDirVS, reflectVS);
		rayPlane.xyz = normalize(cross(reflectVS, vRight));
		rayPlane.w = dot(rayPlane.xyz, PosVS);

		// Iterate over the HDR texture searching for intersection
		for (int nCurStep = 0; nCurStep < nNumSteps; nCurStep++)
		{

			// Sample from depth buffer
			float curDepth = g_DepthMap.SampleLevel(samPoint, ssSampPos, 0.0).x;

			float curDepthLin = ConvertZToLinearDepth(curDepth);
			float3 curPos = CalcViewPos(pin.PosCS.xy + csReflect.xy * ((float)nCurStep + 1.0), curDepthLin);

			// Find the intersection between the ray and the scene
			// The intersection happens between two positions on the oposite sides of the plane
			if (rayPlane.w >= dot(rayPlane.xyz, curPos) + g_DepthBias)
			{
				// Calculate the actual position on the ray for the given depth value
				float3 vsFinalPos = PosVS + (reflectVS / abs(reflectVS.z)) * abs(curDepthLin - PosVS.z + g_DepthBias);
				float2 csFinalPos = vsFinalPos.xy / g_PerspectiveValues.xy / vsFinalPos.z;
				ssSampPos = csFinalPos.xy * float2(0.5, -0.5) + 0.5;



				// Get the HDR value at the current screen space location
				reflectColor.xyz = g_DiffuseMap.SampleLevel(samPoint, ssSampPos, 0.0).xyz;

				// Fade out samples as they get close to the texture edges
				float edgeFade = saturate(distance(ssSampPos, float2(0.5, 0.5)) * 2.0 - g_EdgeDistThreshold);

				// Calculate the fade value
				reflectColor.w = min(viewAngleFade, 1.0 - edgeFade * edgeFade);

				// Apply the reflection sacle
				reflectColor.w *= g_ReflectionScale;

				// Advance past the final iteration to break the loop
				nCurStep = nNumSteps;

			}

			// Advance to the next sample
			ssSampPos += ssStep;
		}
	}

	return reflectColor;
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

	if (isSSR < 0.9f)
	{
		g_OutputMap[globalCoord] = diffuseColor;
		return;
	}

	float3 normalW = g_NormalMap.SampleLevel(samPoint, texCoord, 0.0f).xyz * 2.0f - 1.0f;
	float3 normalV = normalize(mul(normalW, (float3x3)g_View));

	float depth = g_DepthMap.SampleLevel(samPoint, texCoord, 0.0f).x;

	float4 PosV = TexToVS(texCoord, depth);
	PosV.z = ConvertZToLinearDepth(depth);
	float4 PosCS = ViewToCS(PosV);
    
	float3 ViewDirV = normalize(PosV.xyz);

	// Calculate the reflected view direction
	float3 ReflectVS = reflect(ViewDirV, normalV);

	// The initial reflection color for the pixel
	float4 reflectColor = float4(0.0, 0.0, 0.0, 0.0);


	// Don't bother with reflected vector above the threshold vector
	if (ReflectVS.z >= g_ViewAngleThreshold)
	{
		// Fade the reflection as the view angles gets close to the threshold
		float viewAngleThresholdInv = 1.0 - g_ViewAngleThreshold;
		float viewAngleFade = saturate(3.0 * (ReflectVS.z - g_ViewAngleThreshold) / viewAngleThresholdInv);

		// Transform the View Space Reflection to clip-space
		float3 vsPosReflect = PosV + ReflectVS;
		float3 csPosReflect = mul(float4(vsPosReflect, 1.0), g_Proj).xyz / vsPosReflect.z;
		float3 csReflect = csPosReflect - PosCS;

		// Resize Screen Space Reflection to an appropriate length.
		float reflectScale = PixelSize / length(csReflect.xy);
		csReflect *= reflectScale;

		// Calculate the first sampling position in screen-space
		float2 ssSampPos = (PosCS + csReflect).xy;
		ssSampPos = ssSampPos * float2(0.5, -0.5) + 0.5;

		// Find each iteration step in screen-space
		float2 ssStep = csReflect.xy * float2(0.5, -0.5);

		// Build a plane laying on the reflection vector
		// Use the eye to pixel direction to build the tangent vector
		float4 rayPlane;
		float3 vRight = cross(ViewDirV, ReflectVS);
		rayPlane.xyz = normalize(cross(ReflectVS, vRight));
		rayPlane.w = dot(rayPlane.xyz, PosV);

		// Iterate over the HDR texture searching for intersection
		for (int nCurStep = 0; nCurStep < nNumSteps; nCurStep++)
		{

			// Sample from depth buffer
			float curDepth = g_DepthMap.SampleLevel(samPoint, ssSampPos, 0.0).x;

			float curDepthLin = ConvertZToLinearDepth(curDepth);
			float3 curPos = CalcViewPos(PosCS.xy + csReflect.xy * ((float)nCurStep + 1.0), curDepthLin);

			// Find the intersection between the ray and the scene
			// The intersection happens between two positions on the oposite sides of the plane
			if (rayPlane.w >= dot(rayPlane.xyz, curPos) + g_DepthBias)
			{
				// Calculate the actual position on the ray for the given depth value
				float3 vsFinalPos = PosV + (ReflectVS / abs(ReflectVS.z)) * abs(curDepthLin - PosV.z + g_DepthBias);
				float2 csFinalPos = vsFinalPos.xy / g_PerspectiveValues.xy / vsFinalPos.z;
				ssSampPos = csFinalPos.xy * float2(0.5, -0.5) + 0.5;



					// Get the HDR value at the current screen space location
					reflectColor.xyz = g_DiffuseMap.SampleLevel(samPoint, ssSampPos, 0.0).xyz;

					// Fade out samples as they get close to the texture edges
					float edgeFade = saturate(distance(ssSampPos, float2(0.5, 0.5)) * 2.0 - g_EdgeDistThreshold);

					// Calculate the fade value
					reflectColor.w = min(viewAngleFade, 1.0 - edgeFade * edgeFade);

					// Apply the reflection sacle
					reflectColor.w *= g_ReflectionScale;

					// Advance past the final iteration to break the loop
					nCurStep = nNumSteps;
				
			}

			// Advance to the next sample
			ssSampPos += ssStep;
		}
	}

	g_OutputMap[globalCoord] = reflectColor;
}



	technique11 SSRTest2
	{
		pass P0
		{
			SetVertexShader(NULL);
			SetPixelShader(NULL);
			SetComputeShader(CompileShader(cs_5_0, main(false)));
		}
	}


	technique11 SSRTest3
	{
		pass P0
		{
			SetVertexShader(CompileShader(vs_5_0, VS()));
			SetGeometryShader(NULL);
			SetPixelShader(CompileShader(ps_5_0, PS()));
		}
	}

