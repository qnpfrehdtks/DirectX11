Texture2DArray g_TexArray;


// =================================
//            SamplerState
// =================================
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;

	AddressU = WRAP;
	AddressV = WRAP;
};

// =================================
//            Cbuffer
// =================================
cbuffer PerFrame
{
	float3 g_EyePosW;
	float g_Index;
	float4x4 g_ViewProj;

	float4 g_Size = float4(6,7,6,6);
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

// =================================
//            Struct
// =================================
struct VertexIn
{
	float3 PosL : POSITION;
	float4 Color : COLOR;
};

struct VertexOut
{
	float3 CenterW : POSITION;
};

struct GeoOut
{
	float4 PosH : SV_POSITION;
	float3 NormalW : NORMAL;
	float2 Tex : TEXCOORD0;
};

// =============================================
//               Vertex Shader
// =============================================
VertexOut main(VertexIn vin)
{
	VertexOut vout;
	vout.CenterW = vin.PosL;

	return vout;
}

// =============================================
//              Geometry Shader
// =============================================

[maxvertexcount(4)]
void GS(point VertexOut gin[1],
	uint primID : SV_PrimitiveID,
	inout TriangleStream<GeoOut> triStream)
{
	float3 toEyeW = g_EyePosW - gin[0].CenterW;

	toEyeW = normalize(toEyeW);

	float3 up = float3(0, 1, 0);
	float3 right = cross(up, toEyeW);
	right = normalize(right);
	up = cross(toEyeW, right);

	float HalfX = g_Size.x;
	float HalfY = g_Size.y;


	float4 Pos[4];
	Pos[0] = float4(gin[0].CenterW + HalfX * right - HalfY * up, 1.0f);
	Pos[1] = float4(gin[0].CenterW + HalfX * right + HalfY * up, 1.0f);
	Pos[2] = float4(gin[0].CenterW - HalfX * right - HalfY * up, 1.0f);
	Pos[3] = float4(gin[0].CenterW - HalfX * right + HalfY * up, 1.0f);

	[unroll]
	for (int i = 0; i < 4; i++)
	{
		GeoOut gout;

		gout.PosH = mul(Pos[i], g_ViewProj);
		gout.NormalW = toEyeW;
		gout.Tex = g_Tex[i];

		triStream.Append(gout);
	}

}
// =============================================
//             Pixel Shader
// =============================================
float4 PS(GeoOut pin) : SV_Target
{
	float4 result = g_TexArray.Sample(samLinear, float3(pin.Tex, g_Index));

	clip(result.w - 0.1f);

	return  result;
}


technique11 SpriteBasic
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(CompileShader(gs_5_0, GS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}