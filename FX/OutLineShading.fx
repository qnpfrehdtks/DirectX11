
cbuffer cPerObject
{

	// 투영 텍스처를 활용하기 위한 행렬
	float4x4 g_ViewToTexSpace; // Proj * Texture
	float4   g_OffsetVectors[14];
	float4   g_FrustumCorners[4];

	float2 g_Resolution = float2(800.0f, 600.0f);

	// Coordinates given in view space.
	float    g_Edgethickness = 1.0f;
	float    g_EdgeStart = 0.2f;
	float    g_EdgeEnd = 2.0f;
	float    g_SurfaceEpsilon = 0.1f;

};

SamplerState samInputImage
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
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


Texture2D g_NormalDepthMap;

struct VertexIn
{
	float3 PosL            : POSITION;
	float3 ToFarPlaneIndex : NORMAL;
	float2 Tex             : TEXCOORD;
};

struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float3 ToFarPlane : TEXCOORD0;
	float2 Tex        : TEXCOORD1;
};


VertexOut main(VertexIn vin)
{
	VertexOut  vout;

	vout.PosH = float4(vin.PosL, 1.0f);
	vout.ToFarPlane = g_FrustumCorners[vin.ToFarPlaneIndex.x].xyz;
	vout.Tex = vin.Tex;

	return vout;

}



float4 PS(VertexOut pin, uniform int g_SampleCount) : SV_Target
{
	//float dx = 1.0f / g_Resolution.x;
 //   float dy = 1.0f / g_Resolution.y;

	//float3 center = g_NormalDepthMap.SampleLevel(samNormalDepth, pin.Tex, 0.0f);

	return g_NormalDepthMap.SampleLevel(samNormalDepth, pin.Tex, 0.0f);
}




technique11 SilhouetteEdge
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0,PS(9)));
	}
}

