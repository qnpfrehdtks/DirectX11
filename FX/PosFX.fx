
#include "LightHelper.fx"

Texture2D g_HDRTex;
StructuredBuffer<float> g_AvgLum;
Texture2D g_BloomTex;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;

};

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;

};

cbuffer FinalPassConstants 
{
	float4x4 g_WorldViewProj;
	float4x4 g_World;
	float4x4 g_TexTransform;
	float4x4 g_ProjInv;
	float g_ProjA;
	float g_ProjB;
	// Tone mapping
	float g_MiddleGrey;
	float g_LumWhiteSqr;
	float g_BloomScale;
}


struct VertexIn
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex : TEXCOORD;

};


struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD0;
};


VertexOut main(VertexIn vin)
{
	VertexOut  vout;
	
	vout.PosH = mul(float4(vin.PosL, 1.0f), (float4x4)g_WorldViewProj);

	vout.Tex = mul(float4(vin.Tex, 0.0f,1.0f), g_TexTransform).xy;

	return vout;

}


static const float3 LUM_FACTOR = float3(0.299, 0.587, 0.114);

float3 ToneMapping(float3 HDRColor)
{
	// Find the luminance scale for the current pixel
	float LScale = dot(HDRColor, LUM_FACTOR);
	LScale *= g_MiddleGrey / 1.0f;
	LScale = (LScale + LScale * LScale / g_LumWhiteSqr) / (1.0 + LScale);

	// Apply the luminance scale to the pixels color
	return HDRColor * LScale;
}

struct POut
{
	float4 Color1    : SV_Target0;
	float4 Color2  : SV_Target1;

};


POut PS(VertexOut pin) : SV_Target
{
	float3 color = g_HDRTex.Sample(samPoint, pin.Tex).xyz;

	color += (g_BloomTex.Sample(samLinear, pin.Tex).xyz * g_BloomScale);

	// Tone mapping
	color = ToneMapping(color);

	POut result;
	result.Color1 = float4(color,1);
	result.Color2 = float4(color,1);

	return result;

}


technique11 PostFXPass
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));

	}

}

