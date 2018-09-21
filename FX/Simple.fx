

cbuffer cPerObject
{
	float4x4 g_World;
	float4x4 g_WorldViewProj;
};


struct VertexIn
{
	float3 PosL : POSITION;
	float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float4 Color : COLOR;
};


VertexOut main(VertexIn vin)
{
	VertexOut  vout;
	vout.PosH = mul( float4(vin.PosL,1.0f), (float4x4)g_WorldViewProj);
	vout.Color = vin.Color;

	return vout;

}

float4 PS(VertexOut pin) : SV_Target
{
	return pin.Color;
}





technique11 Light1
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));

	}
}

