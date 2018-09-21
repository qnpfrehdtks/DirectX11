//=============================================================================
// BuildShadowMap.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Effect used to build the shadow map.
//
// A lot of code is copy and pasted from DisplacementMap.fx.  When drawing 
// depth to shadow map, we need to tessellate the geometry the same way
// when rendering from the eye so that the shadow map records the same
// geometry the eye sees.
//=============================================================================

cbuffer cbPerFrame
{
	float3 g_EyePosW;

	float g_HeightScale;
	float g_MaxTessDistance;
	float g_MinTessDistance;
	float g_MinTessFactor;
	float g_MaxTessFactor;
};

cbuffer cbPerObject
{
	float4x4 g_ShadowGenMat[6];
	float4x4 g_World;
	float4x4 g_WorldInvTranspose;
	float4x4 g_ViewProj;
	float4x4 g_WorldViewProj;
	float4x4 g_TexTransform;
};

cbuffer cbSkinned
{
	float4x4 g_BoneTransform[200];

};


// Nonnumeric values cannot be added to a cbuffer.
Texture2D g_DiffuseMap;
Texture2D g_NormalMap;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
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


struct VertexIn
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float2 Tex      : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex  : TEXCOORD;
};


VertexOut main(SkinVertex vin)
{
	VertexOut vout;
	float3 posL = float3(0, 0, 0);


	float weight[4];

	weight[0] = vin.Weights.x;
	weight[1] = vin.Weights.y;
	weight[2] = vin.Weights.z;
	weight[3] = 1.0 - weight[0] - weight[1] - weight[2];

	for (int i = 0; i < 4; i++)
	{
		int idx = vin.BoneIndices[i];
		float4x4 boneTranspose = g_BoneTransform[idx];

		posL += mul(float4(vin.PosL, 1.0f), boneTranspose).xyz * weight[i];
	}

	vout.PosH = mul(float4(posL, 1.0f), g_WorldViewProj);
	vout.Tex = vin.Tex;

	return vout;
}

float4 PS(VertexOut pin) : SV_TARGET
{

	return 0.0f;

}



RasterizerState Depth
{
	// [From MSDN]
	// If the depth buffer currently bound to the output-merger stage has a UNORM format or
	// no depth buffer is bound the bias value is calculated like this: 
	//
	// Bias = (float)DepthBias * r + SlopeScaledDepthBias * MaxDepthSlope;
	//
	// where r is the minimum representable value > 0 in the depth-buffer format converted to float32.
	// [/End MSDN]
	// 
	// For a 24-bit depth buffer, r = 1 / 2^24.
	//
	// Example: DepthBias = 100000 ==> Actual DepthBias = 100000/2^24 = .006

	// You need to experiment with these values for your scene.
	DepthBias = 100000;
	DepthBiasClamp = 0.0f;
	SlopeScaledDepthBias = 1.0f;
};

technique11 ShadowAniCubeMap
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));

		SetRasterizerState(Depth);
	}
}

technique11 ShadowCubeMap
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));

		SetRasterizerState(Depth);
	}
}