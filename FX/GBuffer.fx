
#include "LightHelper.fx"


// =============================================================
//                        Constatnt Buffer
// =============================================================
cbuffer cbGs
{
	float4x4 g_ViewProj;
	
	float3 g_EyePosW;
};

cbuffer PerObj
{
	float4 g_Size = float4(2,2, 1, 2);

	float2 g_Tex[4] =
	{
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};
};

cbuffer PerParallax
{
	float g_HeightScale;
	float g_MinSample = 70;
	float g_MaxSample = 85;
};


cbuffer cbPerObj
{
	float4x4 g_World;
	float4x4 g_WorldViewProj;
	float4x4 g_ShadowTransform;
	float4x4 g_WorldInvTranspose;
	float4x4 g_BoneTransform[100];
};

cbuffer cbPerSpec
{
	float g_SpecExp;
	float g_SpecInt;
	bool g_isBumped;
	bool g_isSpec;
	bool g_isMask;
	bool g_isPOM;
	bool g_isShadow;
	bool g_isSSR;
};

// =============================================================
//          Diffuse Texture와 Texture Sample State 정의.
// =============================================================

Texture2D    g_DiffuseMap;
Texture2D    g_NormalMap;
Texture2D    g_SpecularMap;
Texture2D    g_MaskMap;
Texture2D    g_HeightMap;
Texture2D    g_ShadowMap;


SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;

};

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;

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


// =============================================================
//                           구조체
// =============================================================

struct GsVertexIn
{
	float3 PosL : POSITION;
	float4 Color : COLOR;
};

struct GsVertexOut
{
	float3 CenterW : POSITION;
	float4 Color : COLOR;
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

struct PointVertexIn
{
	float3 PosL : POSITION;
};

struct VertexIn
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float4 PosShadowH : POSITION;
	//float3 PosV : POSITION;
	float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
	float2 Tex : TEXCOORD;
	float4 Color : COLOR;
};

struct SkyVertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosL : TEXCOORD;
};

struct POMVertexOut
{
	float4 PosH : SV_POSITION;
	float4 PosShadowH : POSITION0;
	float3 NormalT :NORMAL;
	float3 NormalW : POSITION2;
	float2 Tex : TEXCOORD;
	float3 EyeT : POSITION1;
	uint NumSamples : BONEINDICES;
};

static const float2 g_SpecPowerRange = { 10.0, 250.0 };


// =============================================================
//                          Vertex Shader
// =============================================================

GsVertexOut GsVS(GsVertexIn vin)
{
	GsVertexOut vout;
	vout.CenterW = vin.PosL;
	vout.Color = vin.Color;

	return vout;
}




VertexOut main(VertexIn vin)
{
	VertexOut vout;

	
	vout.PosH = mul(float4(vin.PosL, 1.0f), g_WorldViewProj);
	vout.NormalW = mul(float4(vin.NormalL, 0.0f), g_WorldInvTranspose).xyz;
	vout.Tex = vin.Tex;
	vout.PosShadowH = mul(float4(vin.PosL, 1.0f), g_ShadowTransform);
	//vout.PosV = mul(vin.PosL, (float3x3)g_WorldView).xyz;
	//vout.TangentW = mul(float4(vin.PosL, 1.0f), g_WorldView).xyz;
	vout.Color = 0;
	return vout;
}


VertexOut noSkinVS(SkinVertex vin)
{
	VertexOut vout;

//	vin.NormalL.z *= -1;
	vout.PosH = mul(float4(vin.PosL, 1.0f), g_WorldViewProj);
	vout.NormalW = mul(float4(vin.NormalL,0.0f), g_WorldInvTranspose).xyz;
	vout.Tex = vin.Tex;
	//vout.PosV = mul(float4(vin.PosL, 1.0f), g_WorldView).xyz;
	vout.PosShadowH = mul(float4(vin.PosL, 1.0f), g_ShadowTransform);
	vout.TangentW = mul(vin.TangentL, (float3x3)g_World).xyz;
	vout.Color = 0;
	return vout;
}





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
	float3 normalL = float4(0, 0, 0, 0);

	// 가중치 만큼 정점과 노말, 탄젠트 벡터를 더한다.
	// Skinned Mesh 를 위해, 관절마다 여러 뼈대의 영향을 받는 Vertex가 존재한다.
	for (int i = 0; i < 4; i++)
	{
		int idx = vin.BoneIndices[i];
		float4x4 transposedMat = transpose(g_BoneTransform[idx]);

		posL += (mul(float4(vin.PosL, 1.0f), transposedMat).xyz * weights[i]);
		normalL += (mul(vin.NormalL, (float3x3)transposedMat) * weights[i]);
		tangentL += (mul(vin.TangentL, (float3x3)transposedMat) * weights[i]);
	}

	vout.PosH = mul(float4(posL, 1.0f), g_WorldViewProj);
	vout.NormalW = mul(normalL, (float3x3)g_WorldInvTranspose);
	vout.TangentW = mul(tangentL, (float3x3)g_World).xyz;
	vout.PosShadowH = mul(float4(posL, 1.0f), g_ShadowTransform);
	//vout.PosV = mul(float4(posL, 1.0f), g_WorldView).xyz;
	vout.Tex = vin.Tex;
	vout.Color = 1;
	return vout;
}

// =============================================================
//                     Gbuffer Pack Function
// =============================================================
struct GBuffer_OUT
{
	float4 DiffuseColorSpecInt : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 SpecPow : SV_TARGET2;
};

GBuffer_OUT PackGbuffe(float3 BaseColor, float3 Normal , float SpecIntensity, float SpecPower, float ViewZ, float ShadowFactor)
{
	GBuffer_OUT gBuffer_OUT;

	gBuffer_OUT.DiffuseColorSpecInt = float4(BaseColor, SpecIntensity);
	// [0,1] 로 정규화.
	gBuffer_OUT.Normal = float4(Normal.xyz, 0) * 0.5f + 0.5f;


	gBuffer_OUT.SpecPow = float4(SpecPower, 0, ShadowFactor, 0.0f);

	return gBuffer_OUT;
}


// =============================================================
//                       Geometry Shader
// =============================================================
[maxvertexcount(4)]
void GS(point GsVertexOut gin[1],
	uint primID : SV_PrimitiveID,
	inout TriangleStream<VertexOut> triStream)
{
	float3 up = float3(0, 1, 0);
	float3 toEyeDir = g_EyePosW - gin[0].CenterW;

	//toEyeDir.y = 0.0f;
	toEyeDir = normalize(toEyeDir);

	float3 right = cross(up, toEyeDir);
	right = normalize(right);
	up = cross(toEyeDir, right);

	float HalfX = g_Size.x * 0.5f;
	float HalfY = g_Size.y * 0.5f;

	// 3 =============== 1
	// |                 |
	// |                 |
	// |                 |
	// |                 |
	// |                 |
	// |                 |
	// 2 --------------- 0

	float4 Pos[4];
	Pos[0] = float4(gin[0].CenterW + HalfX * right - HalfY * up, 1.0f);
	Pos[1] = float4(gin[0].CenterW + HalfX * right + HalfY * up, 1.0f);
	Pos[2] = float4(gin[0].CenterW - HalfX * right - HalfY * up, 1.0f);
	Pos[3] = float4(gin[0].CenterW - HalfX * right + HalfY * up, 1.0f);


	//GeoOut
	//{
	//float4 PosH : SV_POSITION;
	//float3 PosV : POSITION;
	//float3 NormalW : NORMAL;
	//float3 TangentW : TANGENT;
	//float2 Tex : TEXCOORD;
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		VertexOut gout;

		gout.PosH = mul(Pos[i], g_ViewProj);
		//gout.PosV = float4(0, 0, 0, 0);
		gout.NormalW = toEyeDir;
		gout.PosShadowH = mul(Pos[i], g_ShadowTransform);
		gout.Tex = g_Tex[i];
		gout.TangentW = gin[0].Color / 255.0f;
		gout.Color = gin[0].Color / 255.0f;
		triStream.Append(gout);
	}

}

// =============================================================
//                          Pixel Shader
// =============================================================

GBuffer_OUT GlowPS(VertexOut pin) : SV_Target
{
	if (g_isMask)
	{
		float color = g_MaskMap.Sample(samLinear, pin.Tex).a;
		clip(color - 0.1f);
	}

    float3 DiffuseColor = pin.Color;
	float3 finalNormalW = (pin.NormalW) ;


	return PackGbuffe(DiffuseColor, finalNormalW, 1.0f , g_SpecExp, 0, 0);
}


GBuffer_OUT PS(VertexOut pin) : SV_Target
{
	if (g_isMask)
	{
		float color = g_MaskMap.Sample(samLinear, pin.Tex).x;
		clip(color - 0.1f);
	}
	//pin.NormalW = normalize(pin.NormalW);

	float3 DiffuseColor = g_DiffuseMap.Sample(samLinear, pin.Tex);

	float ShadowFactor = 1.0f;

	if (g_isShadow) {
	     ShadowFactor = ShadowFactor = CalcShadowFactorWithOutPCF(samPoint, g_ShadowMap, pin.PosShadowH);
	}
	else  ShadowFactor = CalcShadowFactorWithPCF(samShadow, g_ShadowMap, pin.PosShadowH);

	float3 finalNormalW = normalize(pin.NormalW);

	if (g_isBumped)
	{
		float3 NormalMapSample = g_NormalMap.Sample(samLinear, pin.Tex).rgb;
		finalNormalW = NormalSampleToWorldSpace(NormalMapSample, pin.NormalW, pin.TangentW);
	}

	float spec = 0.0f;
	if (g_isSpec)
	{
		spec = g_SpecularMap.Sample(samLinear, pin.Tex).x;
	}

	return PackGbuffe(DiffuseColor, finalNormalW, spec, g_SpecExp, 0, ShadowFactor);
}


// =============================================
//               POM Gbuffer VS
// =============================================
POMVertexOut POMVS(SkinVertex vin)
{
	POMVertexOut vout;
	if (!g_isPOM)
	{
	    VertexOut temp = noSkinVS(vin);
		vout.PosH = temp.PosH;
		vout.NormalT = temp.NormalW;
		vout.EyeT = temp.TangentW;
		vout.NumSamples = 0;
		vout.PosShadowH = temp.PosShadowH;
		vout.Tex = temp.Tex;

		return vout;
	}

	float4 PosH = mul(float4(vin.PosL, 1.0f), g_WorldViewProj);
	float4 ShadowPosH = mul(float4(vin.PosL, 1.0f), g_ShadowTransform);
	float3 PosW = mul(float4(vin.PosL, 1.0f), g_World).xyz;
	float3 NormalW = mul(float4(vin.NormalL, 0.0f), g_WorldInvTranspose).xyz;
	float3 TangentW = mul(vin.TangentL, (float3x3)g_World).xyz;

	float3 fromEyeDirW = PosW - g_EyePosW;

	NormalW = normalize(NormalW);
	//fromEyeDirW = normalize(fromEyeDirW);

	// Sample 할 (Detail) 수준을 정하는 공식. 눈과 노말값이 얼마나 일치하느냐에 따라 
	uint numSamples = (uint)lerp(g_MinSample, g_MaxSample, dot(fromEyeDirW, NormalW));


	// Bitangent 와 Tangent벡터를 이용하여 Tangent Space Transformation Matrix를 만들어 준다.
	float3 N = normalize(NormalW);
	float3 T = normalize(TangentW);
	float3 B = cross(N, T);

	float3x3 tangentToWorldSpace;
	tangentToWorldSpace[0] = T;
	tangentToWorldSpace[1] = B;
	tangentToWorldSpace[2] = N;

	// 회전 행렬이므로, Transpose 함수를 통해 역행렬을 만들어 준다.
	float3x3 worldToTangentSpace = transpose(tangentToWorldSpace);

	vout.PosH = PosH;
	vout.PosShadowH = ShadowPosH;
	vout.NormalT = mul(NormalW, worldToTangentSpace);
	vout.NormalW = NormalW;
	vout.EyeT = mul(fromEyeDirW, worldToTangentSpace);
	vout.NumSamples = 30;
	vout.Tex = vin.Tex;
	return vout;
}
// =============================================
//               POM Gbuffer PS
// =============================================

GBuffer_OUT POMPS(POMVertexOut pin) :SV_Target
{

	if (g_isPOM)
	{
		float3 fromEyeT = pin.EyeT;

		// 최대 시차 오프셋의 크기 설정.
		float ParallaxLimit = -length(fromEyeT.xy) * rcp(fromEyeT.z);
		ParallaxLimit *= g_HeightScale;

		// Offset 계산을 위해서 FromEye vector의 방향을 정규하하여 사용하자.
		float2 OffsetDir = normalize( fromEyeT.xy);
		// maximum parallax offset를 오프셋의 스케일을 조정해준다.
		float2 MaxOffset = OffsetDir * ParallaxLimit;


		// Occulusion Mapping 을 위해서 높이를 같은 레이어의 크기만큼 나누고, 표면의 상단(1.0f) 부터 시작해서 0.0f까지 측정해나간다.
		// 0.0f 까지 내려가면서 단계별로 높이값을 낮출 필요가 있는데, 그 높이값 단계를 여기서 결정.
		//
		int SampleNum = pin.NumSamples;
		float StepSize = 1.0f *rcp((float)SampleNum);


		// Texture 좌표 기울기를 수동으로 계산해내야 함.. 
		float2 dx = ddx(pin.Tex);
		float2 dy = ddy(pin.Tex);

		// 이제 EyeVector와 Height Map과 실제 교차하는 지점을 찾기위해 //
		// Parallax Occulsion 방법을 사용
		//만약 교차하는 지점을 찾으면 Loop 를 종료함.
		// 교차하지 않ㅇ면 EyeVector 방향으로 EyeLimit 값을 더하고, 그 더한 좌표를 HeightMap에서 추출하여 실제 높이를 찾아가야 함,
		// 교차가 일어날려면, 

		float CurHeightLayer = 1.0f; // step마다 진행할 높이값의 크기

		float CurSampledHeight = 1.0f; // 현재 단계에서 뽑아낸 높이값.
		float LastSampledHeight = 1.0f; // 저번 단계에서 뽑아낸 높이값,

		float2 CurOffset = float2(0, 0); // 현재 Eye Vector의 오프셋
		float2 LastOffset = float2(0, 0); // 지난 단계에서의 Eye Vector의 오프셋
		int CurSampleNum = 0; // 현재 Sample 진행한 수.

		while (CurSampleNum < SampleNum) // Sample 만큼 진행할 것.
		{
			CurSampledHeight = g_HeightMap.SampleGrad(samLinear, pin.Tex + CurOffset, dx, dy);

			// 해당 레이어 높이값보다 추출한 높이값이 더클 경우.
			// 지금 추출한 높이값과 지난 샘플 높이값, 지금 샘플 높이값 사이의 보간을 통해 정확한 Offset Vector값을
			// 계산해낸다. // 보간이 중요 
			if (CurHeightLayer < CurSampledHeight)
			{
				float delta1 = (CurSampledHeight - CurHeightLayer);
				float delta2 = (CurHeightLayer + StepSize) - LastSampledHeight;
				float ratio = delta1 * rcp((delta1 + delta2));
				CurOffset = (ratio)* LastOffset + (1.0f - ratio) * CurOffset;
				CurSampleNum = SampleNum + 1;
			}
			else
			{
				CurSampleNum++;
				CurHeightLayer -= StepSize;
				CurOffset += (StepSize * MaxOffset);
				LastOffset = CurOffset;
				LastSampledHeight = CurSampledHeight;
			}
		}

		float2 resultTex = pin.Tex + CurOffset;
		float4 resultNormal = g_HeightMap.Sample(samLinear, resultTex);
		float4 resultColor = g_DiffuseMap.Sample(samLinear, resultTex);
		float specIntensity = g_SpecularMap.Sample(samLinear, resultTex);

		//resultNormal = normalize(resultNormal);
		resultNormal = resultNormal * 2.0f - 1.0f;

		// 이 값들을 Gbuffer에 넣어두어, Deferred Rendering에 활용함.
		// Parameter.
		return PackGbuffe(resultColor, resultNormal, specIntensity, g_SpecExp, 0,0);
	}


     VertexOut temp;
	 temp.PosH = pin.PosH;
	 temp.Tex = pin.Tex;
	 temp.PosShadowH = pin.PosShadowH;
	 temp.NormalW = pin.NormalT;
	 temp.TangentW = pin.EyeT;
	 temp.Color = 0;

	 return PS(temp);

}



// =============================================================
//                          Pass
// =============================================================
technique11 GbufferBasic
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, SkinMain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 GBufferGlow
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, GsVS()));
		SetGeometryShader(CompileShader(gs_5_0, GS()));
		
		SetPixelShader(CompileShader(ps_5_0, GlowPS()));
	}
}

technique11 GbufferSkin
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, SkinMain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 GbufferBasicNormal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, noSkinVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 GbufferPOM
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, POMVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, POMPS()));
	}
}

technique11 GbufferSkinNormal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, SkinMain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}


