
#include "LightHelper.fx"


Texture2D g_TextureMap;


cbuffer cPerObject
{
	float4x4 g_World;
	float4x4 g_WorldView;
	float4x4 g_WorldViewProj;
	float4x4 g_TexTransform;
	float4x4 g_WorldInvTransposeView;
	float4x4 g_WorldInvTranspose;
};



struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};


struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float3 PosV    : TEXCOORD0;
	float3 NormalV : TEXCOORD1;
	float2 Tex     : TEXCOORD2;
};



VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), g_WorldViewProj);
	vout.NormalV = mul(float4(vin.NormalL, 1.0f), g_WorldInvTransposeView).xyz;
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), g_TexTransform).xy;
	vout.PosV = mul(float4(vin.PosL, 1.0f), g_WorldInvTransposeView).xyz;


	return vout;
}

float4 ClampPS(VertexOut pin) : SV_Target
{
	pin.NormalV = normalize(pin.NormalV);
    pin.NormalV = float3(pin.NormalV * 0.5 + 0.5);
	float depthValue = pin.PosH.z / pin.PosH.w;
    return  float4(pin.NormalV, depthValue);
}


float4 PS(VertexOut pin) : SV_Target
{
	pin.NormalV = normalize(pin.NormalV);
    //float depthValue = pin.PosH.z / pin.PosH.w;
	return  float4(pin.NormalV, pin.PosH.z);
}

technique11 NormalDepth
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}

}

technique11 ClampedNormalDepth
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, ClampPS()));

	}

}