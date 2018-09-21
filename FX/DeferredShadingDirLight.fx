
#include "LightHelper.fx"


Texture2D g_DepthMap;
Texture2D g_DiffuseSpecIntMap;
Texture2D g_NormalMap;
Texture2D g_SpecPowMap;
Texture2D g_ShadowMap;
Texture2D g_SSAOMap;

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

SamplerComparisonState samShadow
{
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	ComparisonFunc = LESS;
};

// ===========================================
//                Constant Buffer
// ===========================================

cbuffer cbPerObj
{
	float4 g_ProjMatProperty;
	float4x4 g_InvProj;
	float4x4 g_InvViewProj;
	float4x4 g_WorldViewProj;

	float4x4 g_ShadowTransform;

}

cbuffer cbPerFrame
{
	float4   g_FrustumCorners[4];
	sDirectLight g_DirLight;
	sPointLight g_PtLight[800];
	float3 g_EyePosW;
	float pad;
	
};

// ===========================================
//           Function & Struct
// ===========================================

float ConvertToLinearDepth( float Depth)
{
	return g_ProjMatProperty.w / (Depth - g_ProjMatProperty.z);
}


struct VertexIn
{
	float3 PosL : POSITION;
	float3 ToFarPlaneIndex : NORMAL;
	float2 Tex : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 ToFarPlane : TEXCOORD0;
	float2 Tex : TEXCOORD1;
};

// ========================================================
//                         Vertex Shader
// ========================================================
VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = float4(vin.PosL, 1.0f);

	vout.ToFarPlane = g_FrustumCorners[vin.ToFarPlaneIndex.x].xyz;
	
	vout.Tex = vin.Tex;

	return vout;
}

float3 GetWorldPos(float2 TexCoord, float depth)
{
	// UV 좌표를 NDC 공간 상의 좌표로 변환한다.
	float4 PosPS = float4(TexCoord.x * 2.0f - 1.0f, (1.0f - TexCoord.y) * 2.0f - 1.0f, depth, 1.0f);

	// Proj의 역행렬과 곱해줘서 View Space 공간 좌표 구해옴.
	float4 WorldPos = mul(PosPS, g_InvViewProj);

	// 정규화 시켜서, 좌표 0~1 값으로
	WorldPos.xyz = WorldPos.xyz / WorldPos.w;

	return WorldPos.xyz;

}

float3 GetViewPos(float2 TexCoord, float depth)
{
	// UV 좌표를 NDC 공간 상의 좌표로 변환한다.
	float4 PosPS = float4(TexCoord.x * 2.0f - 1.0f, (1.0f - TexCoord.y) * 2.0f - 1.0f, depth, 1.0f);

	// Proj의 역행렬과 곱해줘서 View Space 공간 좌표 구해옴.
	float4 ViewPos = mul(PosPS, g_InvProj);

	// 정규화 시켜서, 좌표 0~1 값으로
	ViewPos.xyz = ViewPos.xyz / ViewPos.w;

	return ViewPos.xyz;

}

// ========================================================
//                      Pixel Shader
// ========================================================

float4 PS(VertexOut pin, 
	uniform bool isDepth = false, 
	uniform bool isDiffuse = false,
	uniform bool isNormal = false,
	uniform bool isSpecPow = false, 
	uniform bool isShadowed = false,
	uniform bool isSSAO = false
) : SV_TARGET
{

	float4 ambient = float4(0.0f,0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 ShadowColor = float3(1.0f, 1.0f, 1.0f);

	// ============================================
	//                UnPack Gbuffer               
	// ============================================

	 // ================================================
	 //                    Depth
	 // ================================================
	// 여기서 가져온 Depth 값은 ProjZ / ProjW 값인 비선형 깊이값을 나타낸다.
	// 참고 :http://ozlael.egloos.com/3226675 // 비선형 깊이 대신 선형 깊이를 계산해내어,
	//  선형 깊이 값과 해당 스크린 픽셀 좌표를 통해 월드 좌표를 알아낸다.
	 float Depth = g_DepthMap.Sample(samPoint, pin.Tex).x;
	 float linearDepth = ConvertToLinearDepth(Depth);

	 if(isDepth) return float4(linearDepth, linearDepth, linearDepth, 1);

	 // ================================================
	 //                Diffuse + SpecInt;
	 // ================================================
	 float4 DiffuseColorSpecInt = g_DiffuseSpecIntMap.Sample(samPoint, pin.Tex);

	 if (isDiffuse) return float4(DiffuseColorSpecInt.xyz, 1);

	 // ================================================
	 //                     Normal
	 // ================================================
	 float3 Normal = g_NormalMap.Sample(samNormalDepth, pin.Tex).xyz;

	// mul(Normal, float3x3(g_View))


	  if (isNormal) return float4( Normal * 2.0f -1.0f, 1);

	  // ================================================
	  //                    Spec Power
	  // ================================================
	  float SpecPow = g_SpecPowMap.Sample(samPoint, pin.Tex).x;

	  // ================================================
	  //        View Space -> World Space Calcul
	  // ================================================
	 // p 값은 실질적인 View Space의 Ray값.
	  // linearDetph = CamZ / FarZ; 앞에서 구한 깊이 값을 통해 World Pos 값 계산.
	  // 선형 깊이 : ViewSpace의 Z 값을 FarZ 로 나눈 값.
	 //float3 PosVS = ( linearDepth ) * pin.ToFarPlane;
	 //float3 PosWS = mul(float4(PosVS,1.0f), g_InvView).xyz;
	  float3 PosWS = GetWorldPos(pin.Tex, Depth);

	 float3 toEyeW =   g_EyePosW - PosWS.xyz;
	 float distanceToEye = length(toEyeW);
	 toEyeW = normalize(toEyeW);
	 // ================================================
	 //                   SSAO Calcul
	 // ================================================
	 float ssaoFactor = 1.0f;

	 if (false)
	 {
		 ssaoFactor = g_SSAOMap.SampleLevel(samLinear, pin.Tex, 0.0f).r;
		 
	 }
	 // =================================================//
	 //                 Global Rim Light
	 // =================================================
	// float4 RimLightCalcul(float3 toEye, float3 Normal, float4 RimLightColor, float Width);
	 float4 RimLight = { 0,0,0,0 };
	  // RimLight = RimLightCalcul(toEyeW, Normal, float4(0.3f, 0.0f, 0.0f, 1), 0.3f);

	  // =================================================//
	  //                 Depth Shadow Mapping
	  // =================================================
	  if (false) {
		  float4 ShadowPos = mul(float4(PosWS,1.0f), (float4x4)g_ShadowTransform);
		  ShadowColor[0] = CalcShadowFactorWithPCF(samShadow, g_ShadowMap, ShadowPos);
	  }
	
	 // ================================================= //
	 //               Direct Light Calcul
	 // =================================================
	 // 함수 매개변수 
	 /*void ComputeDirectionLightDeferred(
		 sDirectLight L,
		 float3 diffuseColor,
		 float3 normal,
		 float3 toEye,
		 float specPow,
		 float specInt,
		 out float4 ambient,
		 out float4 diffuse
	 )*/

	 float4 A, D, S;

	 ComputeDirectionLightDeferred(
		 g_DirLight,
		 float4(DiffuseColorSpecInt.xyz,1.0f),
		 Normal, 
		 toEyeW,
		 SpecPow,
		 DiffuseColorSpecInt.w,
		 A, D, S);

	 ambient += A;
	 diffuse += D * ShadowColor[0];
	 spec += S * ShadowColor[0];


	 for (int i = 0; i < 800; i++)
	 {
		 ComputePointLightDeferred(
			 g_PtLight[i],
			 PosWS,
			 float4(DiffuseColorSpecInt.xyz, 1.0f),
			 Normal,
			 toEyeW,
			 SpecPow,
			 DiffuseColorSpecInt.w,
			 A, D, S );

		 ambient += A * ssaoFactor;
		 diffuse += D * ShadowColor[0] * ssaoFactor;
		 spec += S * ShadowColor[0];
	 }

	 float4 resultLight = (ambient + diffuse) + spec;
	 resultLight.w = 1.0f;
	// resultLight.xyz += RimLight.xyz;

	 return resultLight;
}


technique11 DeferredDirLight
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 DeferredDepth
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(true)));
	}
}


technique11 DeferredDiffuse
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(false, true)));
	}
}


technique11 DeferredNormal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(false, false, true)));
	}
}


technique11 DeferredDirShadow
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(false, false, false, false, true)));
	}
}


technique11 DeferredDirSSAO
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(false, false, false, false, true, true)));
	}
}
