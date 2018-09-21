// 
// Source나 원리는 DirectX11 책 참고.

// =========================================
//                  Texture
// =========================================
Texture2D g_NormalMap;
Texture2D g_DepthMap;
Texture2D g_InputMap;

// =========================================
//              Constant Buffer
// =========================================
cbuffer cbPerFrame
{
	float4x4 g_View;
	float g_TexelWidth;
	float g_TexelHeight;
	float g_TexelWidt = 0;
	float g_TexelHeigh = 0;
	float4 g_ProjAB;
};

cbuffer cbSettings
{

	float g_Weight[11] =
	{
		0.002216,
		0.008764,
		0.026995,
		0.064759,
		0.120985,
		0.176033,
		0.199471,
		0.176033,
		0.120985,
		0.064759,
		0.026995
	};
};

cbuffer cbFixed
{
	static const int g_BlurRadius = 5;
};


// =========================================
//                  Struct
// =========================================
struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float2 Tex   : TEXCOORD;
};

// ==========================================
//               SamplerState
// =========================================
SamplerState samDepthNormal
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};


SamplerState samInputImage
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

float LinearDepth(float depth) {
	return g_ProjAB.y / (depth - g_ProjAB.x);
}
// =========================================
//               Function
// =========================================
float3 GetNormal(float2 texCoord)
{
	float3 n = g_NormalMap.SampleLevel(samDepthNormal, texCoord, 0.0f).xyz * 0.5f - 0.5f;

	n = mul(n, (float3x3)g_View).xyz;
	n = normalize(n);

	return n;
}

float GetDetph(float2 texCoord)
{
	float d  = g_DepthMap.SampleLevel(samInputImage, texCoord, 0.0f).x;
	d = LinearDepth(d);
	return d;
}

float4 GetInputImageColor(float2 texCoord)
{
	float4 c = g_InputMap.SampleLevel(samInputImage, texCoord, 0.0f);
	return c;
}

// ========================================
//                  V  S
// ========================================
VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = float4(vin.PosL,  1.0f);
	vout.Tex = vin.Tex;

	return vout;
}

// ========================================
//                  P  S
// ========================================
float4 PS(VertexOut pin, uniform bool isVertical) : SV_Target
{
	float2 TexOffset = float2(0.0f,0.0f);

    if (isVertical) 
		TexOffset = float2(g_TexelWidth, 0.0f);
	else 
	{
		TexOffset = float2(0.0f, g_TexelHeight);
	}

	float4 originalColor = GetInputImageColor(pin.Tex);
	float4 resultColor = g_Weight[5] * originalColor;

	float3 centerNormal = GetNormal(pin.Tex);
	float centerDepth = GetDetph(pin.Tex);

	float totalWeight = g_Weight[5];

	for (int i = -g_BlurRadius; i <= g_BlurRadius; i++) {

		if (i == 0)  continue;

		float2 curTex = pin.Tex + (i * TexOffset);

		float3 neighborNormal = GetNormal(curTex);
		float neighborDepth = GetDetph(curTex);


		if (dot(centerNormal, neighborNormal) >= 0.8f &&
			abs(neighborDepth - centerDepth ) <= 0.1f) {

			float weight = g_Weight[g_BlurRadius + i];
			resultColor += (GetInputImageColor(curTex) * weight);
			totalWeight += weight;

			//return originalColor;

		}
	}

	//return float4(1, 1, 0, 0);
	//return originalColor;
	return resultColor / totalWeight;
	

}

technique11 EdgeBlurHorz
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(false)));
	}
}

technique11 EdgeBlurVert
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(true)));
	}
}


