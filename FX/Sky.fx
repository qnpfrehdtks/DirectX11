cbuffer cPerObject
{
	float4x4 g_WorldViewProj;
};


struct VertexIn
{
	float3 PosL : POSITION;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosL : TEXCOORD;
};

TextureCube g_CubeMap;

SamplerState samTriLinearSam
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

VertexOut main(VertexIn vin)
{
	VertexOut  vout;
	vout.PosL = vin.PosL;
	vout.PosH = mul(float4(vin.PosL, 1.0f), g_WorldViewProj).xyww;

	return vout;

}

float4 PS(VertexOut pin) : SV_Target
{
	return g_CubeMap.Sample(samTriLinearSam, pin.PosL);
}


RasterizerState NoCull
{
	CullMode = None;
};

DepthStencilState LessEqualDSS
{
	// Make sure the depth function is LESS_EQUAL and not just LESS.  
	// Otherwise, the normalized depth values at z = 1 (NDC) will 
	// fail the depth test if the depth buffer was cleared to 1.
	DepthFunc = LESS_EQUAL;
};


technique11 SkyRender
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));

		SetRasterizerState(NoCull);
		SetDepthStencilState(LessEqualDSS, 0);

	}
}


