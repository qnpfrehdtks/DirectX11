cbuffer cbPerFrame
{
	float g_Edge;
	float g_Progress;
	float g_EdgeRange;
	float4x4 g_TexTransform;
	float4x4 g_WorldViewProj;
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;

};

Texture2D g_DissolveTex;
Texture2D g_DissolveColorTex;
Texture2D g_DiffuseTex;

struct VertexIn
{
	float4 PosL : POSITION;
	float2 Tex : TEXCOORD;
	float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float4 Color : COLOR;
};


VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(vin.PosL, g_WorldViewProj);
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), g_TexTransform);
	vout.Color = vin.Color;
	vout.Color.a *= g_Progress;

	return vout;
}


float4 PS(VertexOut pin) : SV_Target
{
	float4 Color = g_DissolveTex.Sample(samLinear, pin.Tex);
	float Progress = pin.Color.a;
	float x = Color.r;

	float edge = lerp( x + g_Edge, x - g_Edge, Progress);
	float alpha = smoothstep(Progress + g_Edge, Progress - g_Edge, edge);

	// Color Around Factor
	float EdgeAround = lerp(x + g_EdgeRange, x - g_EdgeRange, Progress);
	EdgeAround = smoothstep(Progress + g_EdgeRange, Progress - g_EdgeRange, EdgeAround);
	EdgeAround = pow(EdgeAround, 2);

	//Edge Around
	Color = g_DiffuseTex.Sample(samLinear, pin.Tex);

	//Edge Around Color
	float3 EdgeColor = g_DissolveColorTex.Sample(samLinear, float2( 1.0f - EdgeAround, 0.0f)).rgb;
	EdgeColor = (Color.rgb + EdgeColor) * EdgeColor * 4.0f;
	Color.rgb = lerp(EdgeColor, Color.rgb, EdgeAround);

	Color.a *= alpha;
	//clip(Color.a - 0.1f);



	return Color;
}

technique11 BasicDissolve
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));

	}

}


