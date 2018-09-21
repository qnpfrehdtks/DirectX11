//=============================================================================
// DebugTexture.fx by Frank Luna (C) 2008 All Rights Reserved.
//
// Effect used to view a texture for debugging.
//=============================================================================

float4x4  g_WorldViewProj;
Texture2D g_Texture;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex  : TEXCOORD;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), g_WorldViewProj);
	vout.Tex = vin.Tex;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return g_Texture.Sample(samLinear, pin.Tex);
}

float4 PS(VertexOut pin, uniform int index) : SV_Target
{
	float4 c = g_Texture.Sample(samLinear, pin.Tex);

	// draw as grayscale
	float g = c[index];
	return float4(g.rrr, 1);
}

technique11 ViewArgbTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 ViewRedTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(0)));
	}
}

technique11 ViewGreenTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(1)));
	}
}

technique11 ViewBlueTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(2)));
	}
}

technique11 ViewAlphaTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(3)));
	}
}