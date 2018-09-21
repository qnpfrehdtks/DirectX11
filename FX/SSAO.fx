
// =============================================
//                 Texture Map
// =============================================
Texture2D g_DepthMap;
Texture2D g_NormalMap;
Texture2D g_RandomVecMap;

// =============================================
//              Constant Buffer
// =============================================
cbuffer cbPerFrame
{
	//float4 g_ProjectProperty;
	float4 g_OffsetPoint[14];
	float4 g_FrustumCorners[4];

	float4x4 g_View;
//	float4x4 g_ViewToTex;
	float4x4 g_InvProj;

	// Coordinates given in view space.
	float    g_OcclusionRadius = 5.0f;
	float    g_OcclusionIntensity = 5.0f;
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
	float4 PosH :SV_POSITION;
	float3 FarPlane : NORMAL;
	float2 Tex : TEXCOORD;
	float3 PosV : POSITION;
};

// ==========================================
//                Functions
// ==========================================
//float3 LinearGetViewPos(float2 TexCoord, float3 FarPlane)
//{
//	float depth = g_DepthMap.SampleLevel(samDepthNormal, TexCoord, 0.0f).x;
//	float linearDepth = g_ProjectProperty.w / (depth - (g_ProjectProperty.z));
//	float3 ViewPos = linearDepth * FarPlane;
//
//	return ViewPos;
//}

float3 GetViewPos(float2 TexCoord){
	// 깊이맵에서 Proj 상의 정규화된 Depth 값을 가져옴.
	float depth = g_DepthMap.SampleLevel(samDepthNormal, TexCoord, 0.0f).x;

	// UV 좌표를 NDC 공간 상의 좌표로 변환한다.
	float4 PosPS = float4(TexCoord.x * 2.0f - 1.0f, (1.0f - TexCoord.y) * 2.0f - 1.0f, depth, 1.0f);

	// Proj의 역행렬과 곱해줘서 View Space 공간 좌표 구해옴.
	float4 CamPos = mul(PosPS, g_InvProj);

	// 정규화 시켜서, 좌표 0~1 값으로
	CamPos.xyz = CamPos.xyz * rcp( CamPos.w);

	return CamPos.xyz;

}

float3 GetNormal(float2 TexCoord)
{
	// Normal Map의 Normal 값은 World space 좌표의 정규화된 벡터임. (Texture 형식은 FLOAT 형. 이유는 w값이 View Space상의 Z값이라.)
	float3 n = g_NormalMap.SampleLevel(samDepthNormal, TexCoord, 0.0f).xyz * 2.0f - 1.0f;

	n = mul(n, (float3x3)g_View);
	n = normalize(n);
	return n;
}

float3 GetRandom(float2 TexCoord)
{
	float3 randVec = g_RandomVecMap.SampleLevel(samRandomVec, 4.0 * TexCoord, 0.0f).rgb;
	randVec = randVec * 2.0f - 1.0f;

	return randVec;
}


float doAmbientOcclusion(in float2 offsetUV, in float2 UV, in float3 p, in float3 norm)
{
	float3 r = GetViewPos(offsetUV + UV);

	// 평면 NearZ와의 SSAO 계산을 방지 하기 위해서 Z값이 NearZ 값 이하면 계산을 안한다.
	if (r.z <= 0.1f) return 0;

	float3 diff = r - p;
	float d = length(diff);

	const float3 v = normalize(diff);

	float NDotV = dot(norm, v);
	return max(0.0, NDotV) *(1.0f  * rcp(d + 1.0f)) * g_OcclusionIntensity;
}

// ==========================================
//                   VS
// =========================================

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = float4(vin.PosL, 1.0f);
	vout.FarPlane = g_FrustumCorners[vin.FarPlaneIndex.x].xyz;
	vout.Tex = vin.Tex;

	return vout;

}

// ==========================================
//                   PS
// =========================================

float4 PS(VertexOut pin, uniform int g_SampleCount) : SV_Target
{
	//const float OcculusionIntesity = 4.0f;
	// ==========================================
	//                Depth Sample
	// ==========================================
	  float3 p = GetViewPos(pin.Tex);
	// ==========================================
	//                 Normal Sample
	// ==========================================
     float3 n = GetNormal(pin.Tex);
	// ==========================================
	//                 Rand Map Sample
	// ==========================================
	 float2 rand = GetRandom(pin.Tex).xy;

	float ao = 0;
	float rad = g_OcclusionRadius * rcp( p.z);
	const float2 offset[4] = { float2(1,0),float2(-1,0), float2(0,1),float2(0,-1) };

	[unroll]
	for (int i = 0; i < g_SampleCount; ++i)
	{
		float2 coord1 = reflect(offset[i].xy, rand.xy) * rad;
		float2 coord2 = float2(coord1.x*0.707 - coord1.y*0.707, coord1.x*0.707 + coord1.y*0.707);

		// ==========================================
		//                 Ao Calcul
		// ==========================================
		ao += doAmbientOcclusion(pin.Tex, coord1*0.25, p, n);
		ao += doAmbientOcclusion(pin.Tex, coord2*0.5, p, n);
		ao += doAmbientOcclusion(pin.Tex, coord1*0.75, p, n);
		ao += doAmbientOcclusion(pin.Tex, coord2, p, n);
	}

	ao *= rcp(g_SampleCount * 4.0f);
	float access = 1.0f - ao;

//	return float4(n, 1.0f);
	return saturate(pow(access,4.0f));
}

technique11 Ssao
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(4)));
	}
}

// 과거의 흔적들....

//// offset 문제도 아니고...
//// randVec 문제? 그것도 아님.
//float3 offset = reflect(g_OffsetPoint[i].xyz, randVec);
//
//float flip = sign(dot(n, offset));
//float3 q = p + flip * offset * g_OcclusionRadius;
//
//// View Proj Tex
//float4 ProjQ = mul(float4(q, 1.0f), g_ViewToTex);
//ProjQ /= ProjQ.w;
//
//// ==========================================
////                Depth Sample
//// ==========================================
//float3 r = GetViewPos(ProjQ, q);
//
//// ==========================================
////                AO Calcul
//// ==========================================
//test += (p.z - r.z) / 2;
//float3 diff = r - p;
//float v = normalize(diff);
//float dp = max(dot(n, v), 0.0f);
//float occlusion = dp * OcclusionFunction(p.z - r.z);
//
//ao += occlusion;