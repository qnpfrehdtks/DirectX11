
#include "LightHelper.fx"

Texture2D g_GlowTexMap;

cbuffer PerFrame
{
	float4x4 g_ViewProj;

	float3 g_EyePosW;
	float4 g_Size;
	float4 g_Color;
};


cbuffer PerObj
{
	float2 g_Tex[4] =
	{
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};



};


struct VertexIn
{
	float3 PosL : POSITION;
};

struct VertexOut
{
	float3 CenterW : POSITION;

};

struct GeoOut
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float3 NormalW : NORMAL;
	float2 Tex : TEXCOORD;
	uint PrimID : SV_PrimitiveID;
};



VertexOut main(VertexIn vin)
{
	VertexOut vout;
	vout.CenterW = vin.PosL;

	return vout;
}




[maxvertexcount(4)]
void GS(point VertexOut gin[1],
	uint primID : SV_PrimitiveID,
	inout TriangleStream<GeoOut> triStream)
{
	float3 up = float3(0, 1, 0);
	float3 toEyeDir = g_EyePosW - gin[0].CenterW;

	toEyeDir.y = 0.0f;
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
	Pos[0] = float4(gin[0].CenterW + HalfX * right - HalfY * up,1.0f);
	Pos[1] = float4(gin[0].CenterW + HalfX * right + HalfY * up, 1.0f);
	Pos[2] = float4(gin[0].CenterW - HalfX * right - HalfY * up, 1.0f);
	Pos[3] = float4(gin[0].CenterW - HalfX * right + HalfY * up, 1.0f);


	//GeoOut
	//{
	////	float4 PosH : SV_POSITION;
	//float3 PosW : POSITION;
	//float3 NormalW : NORMAL;
	//float2 Tex : TEXCOORD;
	//uint PrimID : SV_PrimitiveID;
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		GeoOut gout;

		gout.PosH = mul(Pos[i], g_ViewProj);
		gout.PosW = Pos[i].xyz;
		gout.NormalW = toEyeDir;
		gout.Tex = g_Tex[i];
		gout.PrimID = i;

		triStream.Append(gout);
	}

}


float4 PS(GeoOut pin) : SV_Target
{
	return g_Color;
}

technique11 GlowBasic
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(CompileShader(gs_5_0, GS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));

	}

}

