
#include "LightHelper.fx"


Texture2D g_DepthMap;
Texture2D g_DiffuseSpecIntMap;
Texture2D g_NormalMap;
Texture2D g_SpecPowMap;


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




cbuffer cbBufferUnPack
{
	float4 g_ProjMatProperty;
	float4x4 g_InvVP;
	float4x4 g_WorldViewProj;
}

cbuffer cbPerFrame
{
	//float4x4 g_ViewToTexSpace; // Proj*Texture
	//float4   g_OffsetVectors[14];
	float4   g_FrustumCorners[4];

	sDirectLight g_DirLight;
	/*sPointLight g_PtLight[4];
	sSpotLight g_SpotLight;
*/
	float3 g_EyePosW;
	float pad;
	//sMaterial g_Mat;
};

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

	vout.PosH = mul(float4(vin.PosL, 1.0f), (float4x4)g_WorldViewProj);
	vout.ToFarPlane = g_FrustumCorners[vin.ToFarPlaneIndex.x].xyz;
	vout.Tex = vin.Tex;

	return vout;
}

// ========================================================
//                      Pixel Shader
// ========================================================

float4 PS(VertexOut pin, 
	uniform bool isDepth = false, 
	uniform bool isDiffuse = false,
	uniform bool isNormal = false,
	uniform bool isSpecPow = false) : SV_TARGET
{

	float4 ambient = float4(0.0f,0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// ============================================
	//                UnPack Gbuffer               
	// ============================================

	 // ================================================
	 //                    Depth
	 // ================================================
	// 여기서 가져온 Depth 값은 ProjZ / ProjW 값인 비선형 깊이값을 나타낸다.
	// 참고 :http://ozlael.egloos.com/3226675 // 비선형 깊이 대신 선형 깊이를 계산해내어,
	//  선형 깊이 값과 해당 스크린 픽셀 좌표를 통해 월드 좌표를 알아낸다.
	  float3 Depth = g_DepthMap.Sample(samPoint, pin.Tex).xyz;

	 float linearDepth = ConvertToLinearDepth(Depth.x);

	 if(isDepth) return float4(linearDepth, linearDepth, linearDepth, 1);


	 // ================================================
	 //                Diffuse + SpecInt;
	 // ================================================
	 float4 DiffuseColorSpecInt = g_DiffuseSpecIntMap.Sample(samPoint, pin.Tex);

	 if (isDiffuse) return float4(DiffuseColorSpecInt.xyz, 1);

	 // ================================================
	 //                     Normal
	 // ================================================
	 float3 Normal = g_NormalMap.Sample(samPoint, pin.Tex).xyz;
	  Normal = normalize(Normal * 2.0 - 1.0);

	  if (isNormal) return float4(Normal, 1);

	  // ================================================
	  //                    Ray Calcul
	  // ================================================

	 float SpecPow = g_SpecPowMap.Sample(samPoint, pin.Tex).x;

	 // p 값은 실질적인 View Space의 Ray값.
	 float3 RayV = linearDepth * pin.ToFarPlane;
	 float3 RayW = mul(float4(RayV, 1.0f) , g_InvVP).xyz;

	 float3 toEyeW = g_EyePosW - RayW;
	 toEyeW = normalize(toEyeW);
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
	 diffuse += D ;
	 spec += S;
		 
	return (ambient + diffuse) + spec;
	//return float4(linearDepth, linearDepth, linearDepth,1);
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

