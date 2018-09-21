
#include "LightHelper.fx"


Texture2D g_TextureMap;
Texture2D g_ShadowMap;
Texture2D g_OutLineMap;
Texture2D g_SpecularMap;
Texture2D g_NormalMap;


Texture2D g_DissolveTex;
Texture2D g_DissolveColorTex;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;

};

RasterizerState ableDepth
{
	CullMode = BACK;
	FrontCounterClockwise = true;
	//	DepthWriteMask = ZERO;
};

RasterizerState DisableDepth
{
	CullMode = FRONT;
	FrontCounterClockwise = TRUE;
	//DepthWriteMask = ZERO;
};



SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4; // 비등방 필터링, 최대 비등방 필터링 정도 큰 값일수록 비용이 비싸진다.

					   // 텍스쳐 좌표 지정모드
					   // WRAP : 순환모드에서 이미지가 정수 경계마다 반복된다.
	AddressU = WRAP;
	AddressV = WRAP;

	//BorderColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	// Border color, Clamp 한정

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

cbuffer cPerFrame
{
	sDirectLight g_DirLight;
	sPointLight g_PtLight[4];
	sSpotLight g_SpotLight;

	float3 g_EyePosW;
	sMaterial g_Mat;

};

cbuffer cbSkinned
{
	float4x4 g_BoneTransform[200];
};

cbuffer cPerObject
{
	float4x4 g_World;
	float4x4 g_WorldInvTranspose;
	float4x4 g_WorldViewProj;
	float4x4 g_ToonWVP;
	float4x4 g_ViewProj;
	float4x4 g_TexTransform;
	float4x4 g_ShadowTransform;
	float4x4 g_OutLineTransform;
	float4x4 g_WorldViewProjTex;

	float4 g_RimLightColor;
	float g_RimLightWidth;

	float g_Edge;
	float g_Progress;
	float g_EdgeRange;
	// The intensity of the diffuse light
};

cbuffer cCartoon
{
	
	float g_EdgeTickness;
	// The intensity of the diffuse light
};


struct VertexIn
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex : TEXCOORD;
	//float3 TangentL   : TANGENT;

};


struct SkinVertex
{
	float3 PosL       : POSITION;
	float3 NormalL    : NORMAL;
	float2 Tex        : TEXCOORD;
	float3 TangentL   : TANGENT;
	float3 Weights    : WEIGHTS;
	int4 BoneIndices : BONEINDICES;
	
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
	float2 Tex : TEXCOORD0;
	float4 ShadowPosH : TEXCOORD1;
	float4 OutLinePosH : TEXCOORD2;
//	float4 Color : COLOR;
};


static const float3 LUM_FACTOR = float3(0.299, 0.587, 0.114);


// Skinned Mesh 용 Vertex 
VertexOut SkinMain(SkinVertex vin)
{
	VertexOut vout;

	float weights[4];

	weights[0] = vin.Weights.x;
	weights[1] = vin.Weights.y;
	weights[2] = vin.Weights.z;
	weights[3] = 1.0f - weights[1] - weights[2] - weights[0];

	float3 posL = float3(0, 0, 0);
	float3 tangentL = float3(0, 0, 0);
	float3 normalL = float3(0, 0, 0);

	// 가중치 만큼 정점과 노말, 탄젠트 벡터를 더한다.
	// Skinned Mesh 를 위해, 관절마다 여러 뼈대의 영향을 받는 Vertex가 존재한다.
	for (int i = 0; i < 4; i++)
	{
		int idx = vin.BoneIndices[i];
		posL +=( mul(float4(vin.PosL, 1.0f), transpose(g_BoneTransform[idx])).xyz * weights[i]);
		tangentL += (mul(vin.TangentL, (float3x3)transpose(g_BoneTransform[idx])) * weights[i]);
		normalL += (mul(vin.NormalL, (float3x3)transpose(g_BoneTransform[idx])) * weights[i]);
	}

	vout.PosW = mul(float4(posL, 1.0f), g_World).xyz;
	vout.PosH = mul(float4(posL, 1.0f), g_WorldViewProj);
	vout.NormalW = mul(normalL, (float3x3)g_WorldInvTranspose);
	vout.TangentW = mul(tangentL, (float3x3)g_World).xyz;

	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), g_TexTransform).xy;
	vout.ShadowPosH = mul(float4(posL, 1.0f), g_ShadowTransform);
	vout.OutLinePosH = mul(float4(posL, 1.0f), g_WorldViewProjTex);

	return vout;
}




VertexOut main(VertexIn vin)
{
	VertexOut  vout;
	vout.PosW = mul(float4(vin.PosL,1.0f),g_World).xyz;
	vout.PosH = mul( float4(vin.PosL,1.0f), (float4x4)g_WorldViewProj);
	vout.NormalW = mul(vin.NormalL, (float3x3)g_WorldInvTranspose);
	vout.TangentW = float3(0, 0, 0);
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), g_TexTransform).xy;
	vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), g_ShadowTransform);
	vout.OutLinePosH = mul(float4(vin.PosL, 1.0f), g_WorldViewProjTex);

	//vout.PosH *= 12.0f;

	return vout;

}

// ===========================================================
//       Ambient 값과 Normal 값만을 이용하여, Rendering
// ===========================================================
float3 CaculAmbient(float3 normal , float3 color, float3 ambientDownColor, float3 ambientRange)
{
	// 노말 값을 [-1, 1] 에서 [0, 1]
	float up = normal.y * 0.5 + 0.5;

	// Ambeint 아래에서 받는 색상과 위에서는 받는 색상과 범위를 곱해서 더함.
	float3 ambient = ambientDownColor + up * ambientRange;

	
	return ambient * color;
}

float4 AmbinetPS(VertexOut pin) : SV_Target
{
	// Diffuse 의 색깔. RGB
	float3 DiffuseColor = float3(1.0f, 1.0f, 1.0f);
	float3 AmbientDownColor = float3(1.0f, 0.0f, 0.0f);
	float3 AmbientRange = DiffuseColor - AmbientDownColor;

	// Calculate the ambient color
	float3 AmbientColor = CaculAmbient(pin.NormalW, DiffuseColor, AmbientDownColor, AmbientRange);

	// Return the ambient color
	return float4(AmbientColor, 1.0);

}




// ========================================================================
//     Dissolve 계산
// ========================================================================

float4 CalDissolve(float DissolveColor, float Progress, float4 DiffuseColor)
{
	float edge = lerp(DissolveColor + g_Edge, DissolveColor - g_Edge, Progress);
	float alpha = smoothstep(Progress + g_Edge, Progress - g_Edge, edge);

	// Color Around Factor
	float EdgeAround = lerp(DissolveColor + g_EdgeRange, DissolveColor - g_EdgeRange, Progress);
	EdgeAround = smoothstep(Progress + g_EdgeRange, Progress - g_EdgeRange, EdgeAround);
	EdgeAround = pow(EdgeAround, 2);


	//Edge Around Color
	float3 EdgeColor = g_DissolveColorTex.Sample(samLinear, float2(1.0f - EdgeAround, 0.0f)).rgb;
	EdgeColor = (DiffuseColor.rgb + EdgeColor) * EdgeColor * 5.0f;

	float4 resultCol = float4(0.0f, 0.0f, 0.0f, 1.0f);
	resultCol.rgb = lerp(EdgeColor, DiffuseColor.rgb, EdgeAround);

	resultCol.a *= alpha;


	return resultCol;
}



// ========================================================================
//     MainPS Shadow + Rim + SSAO + Lights 등 다양한 연산을 통해 출력
// ========================================================================
float4 PS(VertexOut pin,
	uniform int PtLightNum,
	uniform bool isUseTexture,
	uniform bool isShadowed,
	uniform bool isRimLight,
	uniform bool isGamma = false,
	uniform bool isDissolve = false,
	uniform bool isBumped = false,
	uniform bool HasSpecMap = false) : SV_Target
{
	// ===============================================================================
	//                              toEye, NormalW Setting
	// ===============================================================================
	   float3 toEye = g_EyePosW - pin.PosW ;
	   float distanceToEye = length(toEye);
	   toEye = toEye / distanceToEye;
	   pin.NormalW = normalize(pin.NormalW);

	   // ===============================================================================
	   //                              amient, diffuse, spec, 변수 설정.
	   // ===============================================================================

	   float4 ambient = float4(0.0f,0.0f, 0.0f, 0.0f);
	   float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	   float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	   float3 ShadowColor = float3(1.0f, 1.0f, 1.0f);
	   float4 RimLightColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//  pin.OutLinePosH /= pin.OutLinePosH.w;
	//  float3 outlineFactor = g_OutLineMap.SampleLevel(samLinear, pin.OutLinePosH.xy, 0.0f).xyz;

     float outlineFactor = 1;
	   if (false) {
		   ShadowColor[0] = CalcShadowFactorWithPCF(samShadow, g_ShadowMap, pin.ShadowPosH);
	   }


	  
	   // ==============================================================================
	   //                              Normal Map Calcul
	   // ==============================================================================
	   float3 finalNormalW = pin.NormalW;
	   float3 NormalMapSample;
	   if (isBumped)
	   {
		   float3 NormalMapSample = g_NormalMap.Sample(samLinear, pin.Tex).rgb;
		   finalNormalW = NormalSampleToWorldSpace(NormalMapSample, pin.NormalW, pin.TangentW);
	   }
	   // ===============================================================================
	   //                              Direct Light Calcul
	   // ===============================================================================
	   float4 A, D, S;

	   ComputeDirectionLight(g_Mat, g_DirLight, finalNormalW, toEye,
			   A, D, S);

		   ambient += A;
		   diffuse += D * ShadowColor[0];
		   spec += S * ShadowColor[0] ;

		// ===============================================================================
		//                              Rim Light Cacul
		// ===============================================================================
		if (true) {
			 RimLightColor = RimLightCalcul(toEye, finalNormalW, g_RimLightColor, g_RimLightWidth);
		}

		// ==============================================================================
		//                              Specular Map Calcul
		// ==============================================================================
	       if (true)
		   {
			   float4 specIntensity = { 1.0f, 1.0f, 1.0f,1.0f };
			   specIntensity = g_SpecularMap.Sample(samLinear, pin.Tex);
			   spec = spec * specIntensity;
		   }

		   // ===============================================================================
		   //                              Point Light Calcul
		   // ===============================================================================
		   [unroll]
		   for (int i = 0; i < 0; i++)
		   {
			   float4 A, D, S;

			   ComputePointLight(g_Mat, g_PtLight[i], pin.PosW, finalNormalW, toEye, A, D, S);
			   ambient += A;
			   diffuse += D * ShadowColor[0];
			   spec += S * ShadowColor[0];
		   }
	   
		  

		 


		// ===============================================================================
		//                             Texture Calcul
		// ===============================================================================
	   float4 texColor = float4(1, 1, 1, 1);

	   if (true)
	   {
		   texColor = g_TextureMap.Sample(samLinear, pin.Tex);
		   clip(texColor.a - 0.1f);
	   }

	   float4 resultLight = texColor * (ambient + diffuse) + spec;
	   resultLight.w = 1.0f;


	   // ==========================================================
	   //                           Dissolve Color
	   // ==========================================================

	   float4 DissolveColor = float4(1, 1, 1, 0);
	   if (true) {
		   float dissolveColor = g_DissolveTex.Sample(samLinear, pin.Tex).x;
		   DissolveColor = CalDissolve(dissolveColor, g_Progress, resultLight);
	   }

	   resultLight = DissolveColor;
	   // ===============================================================================
	   //                             Result Light 
	   // ===============================================================================

	   resultLight.xyz += RimLightColor;
	
	   return resultLight;

}



struct VertexOut2
{
	float4 PosH : SV_POSITION;
};

VertexOut2 OutlineVertexShader(VertexIn vin)
{
	VertexOut2  vout;
	
	float3 newPos = vin.PosL + vin.NormalL * 0.5f;

	vout.PosH = mul(float4(newPos, 1.0f), g_WorldViewProj);

	return vout;

}

VertexOut2 SkinOutlineVertexShader(SkinVertex vin)
{
	float weights[4];
	weights[0] = vin.Weights[0];
	weights[1] = vin.Weights[1];
	weights[2] = vin.Weights[2];
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	VertexOut2  vout;
	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	//float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; i++)
	{
		int idx = vin.BoneIndices[i];
		posL += (mul(float4(vin.PosL, 1.0f), transpose(g_BoneTransform[idx])).xyz * weights[i]);
	//	tangentL += (mul(vin.TangentL, (float3x3)transpose(g_BoneTransform[idx])) * weights[i]);
		normalL += (mul(vin.NormalL, (float3x3)transpose(g_BoneTransform[idx])) * weights[i]);
	}

	float2 xz = normalize(posL.xz);
	float3 norm = mul(float3(xz.x, 1.,xz.y), (float3x3)(g_WorldViewProj)).xyz;

	vout.PosH = mul(float4(posL, 1.0f), g_WorldViewProj);
	vout.PosH.xy += norm.xy * vout.PosH.z * 0.05f;

	return vout;

}

float4 OutLinePS(VertexOut2 pin) : SV_Target
{
	return float4(0,0,0,1);
}

technique11 SkinDirLight
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, SkinMain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(0, true, true, true)));

	}

}

technique11 SkinDirLightNoRim
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, SkinMain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(0, true, true, false)));

	}

}

technique11 SkinDirLightNoRimNoBumped
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, SkinMain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(0, true, true, false,false,false,false,false)));

	}

}

technique11 SkinDirLightBumped
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, SkinMain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(0, true, true, false, false, false, true, false)));

	}

}



technique11 SkinDirLightToon
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, SkinMain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(0, true, true, true)));
		SetRasterizerState(DisableDepth);
	}
	pass P1
	{
		SetVertexShader(CompileShader(vs_5_0, SkinOutlineVertexShader()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, OutLinePS()));
		SetRasterizerState(ableDepth); 
	//	ableDepth
	}

}

//uniform int PtLightNum,
//uniform bool isUseTexture,
//uniform bool isShadowed,
//uniform bool isRimLight,
//uniform bool isGamma = false,
//uniform bool isDissolve = false,
//uniform bool isBumped = false,
//uniform bool HasSpecMap = false

technique11 SkinDirLightNormal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, SkinMain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(0, true, false, true, false, false, true, false)));
	}
}

technique11 SkinDirLightNormalSpec
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, SkinMain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(0, true, true, true, false, true, true, true)));
	}
}


technique11 DirLight
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(4, false, true, false)));
		
	}
	
}

technique11 TexDirLight
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(4, true, true, false,false)));

	}

}

technique11 GammaTexDirLight
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(4, true, true, true, true)));

	}

}


technique11 OutLine
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(4, false, false, false)));
     	//SetRasterizerState(DisableDepth);
	  	SetRasterizerState(ableDepth);
	//lMode = CW;
	}
	pass P1
	{
		SetVertexShader(CompileShader(vs_5_0, OutlineVertexShader()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, OutLinePS()));
		SetRasterizerState(DisableDepth);
		//CullMode = CCW;
	}
}


technique11 SimpleLight
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(4, false, true, false)));
	}
}

technique11 RimBasicLight
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(4, true, true, true)));
	}
}


technique11 BasicDissolve
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(4, true, true, true, true, true)));
	}
}



