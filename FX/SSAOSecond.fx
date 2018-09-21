
// =============================================
//                 Texture Map
// =============================================
Texture2D g_NormalMap;
Texture2D g_DepthMap;
Texture2D g_RandomVecMap;

cbuffer cbPerObj
{
	//float random_size;
	float g_sample_rad = 0.5f;
	float g_intensity = 1.0f;
	float g_scale = 1.0f;
	float g_bias = 1.0f;
};

cbuffer cbPerFrame
{
	float4x4 g_View;
	float4x4 g_InvProj;
};

// ==========================================
//               SamplerState
// =========================================
SamplerState samDepthNormal
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	// Set a very far depth value if sampling outside of the NormalDepth map
	// so we do not get false occlusions.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 1e5f);
};

SamplerState samRandomVec
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};


// ==========================================
//                Struct
// =========================================
struct VertexIn
{
	float3 PosL : POSITION;
	float3 FarPlaneIndex : NORMAL;
	float2 Tex : TEXCOORD;
};

struct VertexOut
{ 
	float4 PosH : SV_POSITION; 
	float2 uv : TEXCOORD0; 
};


// ==========================================
//              Function
// =========================================
float3 GetViewPos(float2 TexCoord)
{
	// Depth Buffer Store Value [ 0,1 ]
	float ViewDepth = g_NormalMap.SampleLevel(samDepthNormal, TexCoord, 0.0f).w;
	float ProjDepth = g_DepthMap.SampleLevel(samDepthNormal, TexCoord, 0.0f).x;
	//if (ViewDepth > 1.01) return float3(0, 1, 0);
	
	float4 clipSpacePosition = float4(TexCoord.x * 2.0 - 1.0, (1.0f - TexCoord.y) * 2.0f - 1.0f, ProjDepth, 1.0f);
	float3 ViewPos = mul(clipSpacePosition, g_InvProj).xyz;
	ViewPos.z = ViewDepth;
	return ViewPos;
}

float3 GetNormal(float2 TexCoord)
{
	float3 n = g_NormalMap.SampleLevel(samDepthNormal, TexCoord, 0.0f).xyz;

	n = n * 2.0f - 1.0f;
	n = mul(n, (float3x3)g_View);
	n = normalize(n);
	return n;

}

float2 getRandom( float2 uv)
{
	return normalize(
		g_RandomVecMap.SampleLevel(samRandomVec, 
			4.0f * uv , 0.0f).xy * 2.0f - 1.0f);
}

float doAmbientOcclusion(in float2 tcoord, in float2 uv, in float3 p, in float3 cnorm)
{
	float3 diff = GetViewPos(tcoord + uv) - p;
	const float3 v = normalize(diff);
	const float d = length(diff)*g_scale;
	return max(0.0, dot(cnorm, v) - g_bias)*(1.0 / (1.0 + d))*g_intensity;
}

// ==========================================
//                      VS
// =========================================
VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.uv = vin.Tex;
	vout.PosH = float4(vin.PosL, 1.0f);

	return vout;
}

// ==========================================
//                    PS
// =========================================
float4 ps(VertexOut i) : SV_Target
{
	return float4(GetViewPos(i.uv), 1.0f);
}

technique11 SsaoSecond
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, ps()));
	}
}