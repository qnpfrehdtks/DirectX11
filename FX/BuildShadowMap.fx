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


// Skinned Mesh �� Vertex 
VertexOut SkinMain(SkinVertex vin)
{
	VertexOut vout;

	float weights[4];

	weights[0] = vin.Weights.x;
	weights[1] = vin.Weights.y;
	weights[2] = vin.Weights.z;
	weights[3] = 1.0f - weights[1] - weights[2] - weights[0];

	float3 posL = float3(0, 0, 0);

	// ����ġ ��ŭ ������ �븻, ź��Ʈ ���͸� ���Ѵ�.
	// Skinned Mesh �� ����, �������� ���� ������ ������ �޴� Vertex�� �����Ѵ�.
	for (int i = 0; i < 4; i++)
	{
		int idx = vin.BoneIndices[i];
		posL += (mul(float4(vin.PosL, 1.0f), transpose(g_BoneTransform[idx])).xyz * weights[i]);
	}

	vout.PosH = mul(float4(posL, 1.0f), g_WorldViewProj);
	vout.Tex = vin.Tex;

	return vout;
}


// Skinned Mesh �� Vertex 
VertexOut noSkinMain(SkinVertex vin)
{
	VertexOut vout;
	vout.PosH = mul(float4(vin.PosL, 1.0f), g_WorldViewProj);
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), g_TexTransform).xy;

	return vout;
}



VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), g_WorldViewProj);
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), g_TexTransform).xy;

	return vout;
}


struct TessVertexOut
{
	float3 PosW       : POSITION;
	float3 NormalW    : NORMAL;
	float2 Tex        : TEXCOORD;
	float  TessFactor : TESS;
};

TessVertexOut TessVS(VertexIn vin)
{
	TessVertexOut vout;

	vout.PosW = mul(float4(vin.PosL, 1.0f), g_World).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)g_WorldInvTranspose);
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), g_TexTransform).xy;

	float d = distance(vout.PosW, g_EyePosW);

	// Normalized tessellation factor. 
	// The tessellation is 
	//   0 if d >= gMinTessDistance and
	//   1 if d <= gMaxTessDistance.  
	float tess = saturate((g_MinTessDistance - d) / (g_MinTessDistance - g_MaxTessDistance));

	// Rescale [0,1] --> [gMinTessFactor, gMaxTessFactor].
	vout.TessFactor = g_MinTessFactor + tess * (g_MaxTessFactor - g_MinTessFactor);

	return vout;
}

struct PatchTess
{
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess : SV_InsideTessFactor;
};

PatchTess PatchHS(InputPatch<TessVertexOut, 3> patch,
	uint patchID : SV_PrimitiveID)
{
	PatchTess pt;

	// Average tess factors along edges, and pick an edge tess factor for 
	// the interior tessellation.  It is important to do the tess factor
	// calculation based on the edge properties so that edges shared by 
	// more than one triangle will have the same tessellation factor.  
	// Otherwise, gaps can appear.
	pt.EdgeTess[0] = 0.5f*(patch[1].TessFactor + patch[2].TessFactor);
	pt.EdgeTess[1] = 0.5f*(patch[2].TessFactor + patch[0].TessFactor);
	pt.EdgeTess[2] = 0.5f*(patch[0].TessFactor + patch[1].TessFactor);
	pt.InsideTess = pt.EdgeTess[0];

	return pt;
}

struct HullOut
{
	float3 PosW     : POSITION;
	float3 NormalW  : NORMAL;
	float2 Tex      : TEXCOORD;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut HS(InputPatch<TessVertexOut, 3> p,
	uint i : SV_OutputControlPointID,
	uint patchId : SV_PrimitiveID)
{
	HullOut hout;

	// Pass through shader.
	hout.PosW = p[i].PosW;
	hout.NormalW = p[i].NormalW;
	hout.Tex = p[i].Tex;

	return hout;
}

struct DomainOut
{
	float4 PosH     : SV_POSITION;
	float3 PosW     : POSITION;
	float3 NormalW  : NORMAL;
	float2 Tex      : TEXCOORD;
};

// The domain shader is called for every vertex created by the tessellator.  
// It is like the vertex shader after tessellation.
[domain("tri")]
DomainOut DS(PatchTess patchTess,
	float3 bary : SV_DomainLocation,
	const OutputPatch<HullOut, 3> tri)
{
	DomainOut dout;

	// Interpolate patch attributes to generated vertices.
	dout.PosW = bary.x*tri[0].PosW + bary.y*tri[1].PosW + bary.z*tri[2].PosW;
	dout.NormalW = bary.x*tri[0].NormalW + bary.y*tri[1].NormalW + bary.z*tri[2].NormalW;
	dout.Tex = bary.x*tri[0].Tex + bary.y*tri[1].Tex + bary.z*tri[2].Tex;

	// Interpolating normal can unnormalize it, so normalize it.
	dout.NormalW = normalize(dout.NormalW);

	//
	// Displacement mapping.
	//

	// Choose the mipmap level based on distance to the eye; specifically, choose
	// the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
	const float MipInterval = 20.0f;
	float mipLevel = clamp((distance(dout.PosW, g_EyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);

	// Sample height map (stored in alpha channel).
	float h = g_NormalMap.SampleLevel(samLinear, dout.Tex, mipLevel).a;

	// Offset vertex along normal.
	dout.PosW += (g_HeightScale*(h - 1.0))*dout.NormalW;

	// Project to homogeneous clip space.
	dout.PosH = mul(float4(dout.PosW, 1.0f), g_ViewProj);

	return dout;
}

// This is only used for alpha cut out geometry, so that shadows 
// show up correctly.  Geometry that does not need to sample a
// texture can use a NULL pixel shader for depth pass.
void PS(VertexOut pin)
{
	float4 diffuse = g_DiffuseMap.Sample(samLinear, pin.Tex);

	// Don't write transparent pixels to the shadow map.
	clip(diffuse.a - 0.15f);
}

// This is only used for alpha cut out geometry, so that shadows 
// show up correctly.  Geometry that does not need to sample a
// texture can use a NULL pixel shader for depth pass.
void TessPS(DomainOut pin)
{
	float4 diffuse = g_DiffuseMap.Sample(samLinear, pin.Tex);

	// Don't write transparent pixels to the shadow map.
	clip(diffuse.a - 0.15f);
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

technique11 BuildShadowMapTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(NULL);

		SetRasterizerState(Depth);
	}
}

technique11 BuildShadowMapSkinTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, SkinMain()));
		SetGeometryShader(NULL);
		SetPixelShader(NULL);

		SetRasterizerState(Depth);
	}
}

technique11 BuildShadowMapNoSkinTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, noSkinMain()));
		SetGeometryShader(NULL);
		SetPixelShader(NULL);

		SetRasterizerState(Depth);
	}
}

technique11 BuildShadowMapAlphaClipTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 TessBuildShadowMapTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, TessVS()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetGeometryShader(NULL);
		SetPixelShader(NULL);

		SetRasterizerState(Depth);
	}
}

technique11 TessBuildShadowMapAlphaClipTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, TessVS()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, TessPS()));
	}
}