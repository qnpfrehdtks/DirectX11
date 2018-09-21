
// =====================================================================
//               ���� ���� ����(Parallax Occulusion Mapping)
// =====================================================================
//https://www.gamedev.net/articles/programming/graphics/a-closer-look-at-parallax-occlusion-mapping-r3262/
// parallax-occlusion-mapping ����. 
//http://sunandblackcat.com/tipFullView.php?l=eng&topicid=28
// �׸� ���� ����.


Texture2D g_HeightMap;
//Texture2D g_NormalMap;
Texture2D g_DiffuseMap;
Texture2D g_SpecularMap;


cbuffer PerObj
{
	float3 g_EyePosW;
	float4x4 g_World;
	float4x4 g_WorldViewProj;
	float g_SpecExp;
};

cbuffer PerParallax
{
	float g_HeightScale;
	float g_MinSample;
	float g_MaxSample;
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;

};


struct SkinVertexIn
{
	float3 PosL       : POSITION;
	float3 NormalL    : NORMAL;
	float2 Tex        : TEXCOORD;
	float3 TangentL   : TANGENT;
	float3 Weights    : WEIGHTS;
	int4 BoneIndices : BONEINDICES;

};



struct VertexOut
{
	float3 PosH : SV_POSITION;
	float3 NormalT :NORMAL;
	float2 Tex : TEXCOORD;
	float3 EyeT : POSITION;
	uint NumSamples : SV_PrimitiveID;
	
};

struct GBuffer_OUT
{
	float4 DiffuseColorSpecInt : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 SpecPow : SV_TARGET2;
};

// =============================================
//           Gbuffer Pack Function
// =============================================
GBuffer_OUT PackGbuffe(float3 BaseColor, float3 Normal, float SpecIntensity, float SpecPower, float ViewZ)
{
	GBuffer_OUT gBuffer_OUT;

	gBuffer_OUT.DiffuseColorSpecInt = float4(BaseColor, SpecIntensity);
	// [0,1] �� ����ȭ.
	gBuffer_OUT.Normal = float4(Normal.xyz, ViewZ) * 0.5f + 0.5f;

	gBuffer_OUT.SpecPow = float4(SpecPower, 0.0f, 0.0f, 0.0f);

	return gBuffer_OUT;
}



// =============================================
//                  Vertex Shader
// =============================================
VertexOut main(SkinVertexIn vin)
{
	VertexOut vout;
    float4 PosH = mul( float4(vin.PosL,1.0f), g_WorldViewProj);
	float3 PosW = mul(float4(vin.PosL, 1.0f), g_World).xyz;
	float3 NormalW = mul(float4(vin.NormalL, 1.0f), g_World);
	float3 TangentW = mul(normalize(vin.TangentL), g_World);


	float3 fromEyeDirW = PosW - g_EyePosW;

	// Sample �� (Detail) ������ ���ϴ� ����. ���� �븻���� �󸶳� ��ġ�ϴ��Ŀ� ���� 
	uint numSamples = (uint)lerp(g_MinSample, g_MaxSample, dot(fromEyeDirW, NormalW));

	// Bitangent �� Tangent���͸� �̿��Ͽ� Tangent Space Transformation Matrix�� ����� �ش�.
	float3 N = normalize(NormalW);
	float3 T = normalize(TangentW - dot(TangentW, N) * N);
	float3 B = cross(N, T);

	float3x3 tangentToWorldSpace;
	tangentToWorldSpace[0] = T;
	tangentToWorldSpace[1] = B;
	tangentToWorldSpace[2] = N;

	// ȸ�� ����̹Ƿ�, Transpose �Լ��� ���� ������� ����� �ش�.
	float3x3 worldToTangentSpace = transpose(tangentToWorldSpace);

	vout.PosH = PosH;
	vout.NormalT = mul(NormalW, worldToTangentSpace);
	vout.EyeT = mul(fromEyeDirW, worldToTangentSpace);
	vout.NumSamples = numSamples;
	vout.Tex = vin.Tex;

	return vout;
}


// =============================================
//                  Pixel Shader
// =============================================
GBuffer_OUT main(VertexOut pin)
{
	float3 fromEyeT = pin.EyeT;

	// �ִ� ���� �������� ũ�� ����.
	float ParallaxLimit = -length( fromEyeT.xy) * rcp(fromEyeT.z);
	ParallaxLimit *= g_HeightScale;

	// Offset ����� ���ؼ� FromEye vector�� ������ �������Ͽ� �������.
	float2 OffsetDir = normalize(fromEyeT.xy);
	// maximum parallax offset�� �������� �������� �������ش�.
	float2 MaxOffset = OffsetDir * ParallaxLimit;


	// Occulusion Mapping �� ���ؼ� ���̸� ���� ���̾��� ũ�⸸ŭ ������, ǥ���� ���(1.0f) ���� �����ؼ� 0.0f���� �����س�����.
	// 0.0f ���� �������鼭 �ܰ躰�� ���̰��� ���� �ʿ䰡 �ִµ�, �� ���̰� �ܰ踦 ���⼭ ����.
	//
	int SampleNum = pin.NumSamples;
	float StepSize = 1.0f *rcp( (float)SampleNum);


	// Texture ��ǥ ���⸦ �������� ����س��� ��.. 
	float2 dx = ddx(pin.Tex);
	float2 dy = ddy(pin.Tex);

	// ���� EyeVector�� Height Map�� ���� �����ϴ� ������ ã������ //
	// Parallax Occulsion ����� ���
	//���� �����ϴ� ������ ã���� Loop �� ������.
	// �������� �ʤ��� EyeVector �������� EyeLimit ���� ���ϰ�, �� ���� ��ǥ�� HeightMap���� �����Ͽ� ���� ���̸� ã�ư��� ��,
	// ������ �Ͼ����, 

	float CurHeightLayer = 1.0f; // step���� ������ ���̰��� ũ��

	float CurSampledHeight = 1.0f; // ���� �ܰ迡�� �̾Ƴ� ���̰�.
	float LastSampledHeight = 1.0f; // ���� �ܰ迡�� �̾Ƴ� ���̰�,
	
	float2 CurOffset = float2(0, 0); // ���� Eye Vector�� ������
	float2 LastOffset = float2(0, 0); // ���� �ܰ迡���� Eye Vector�� ������
	int CurSampleNum = 0; // ���� Sample ������ ��.
	 
	while (CurSampleNum < SampleNum) // Sample ��ŭ ������ ��.
	{
		CurSampledHeight = g_HeightMap.SampleGrad(samLinear, pin.Tex + CurOffset, dx, dy);

		// �ش� ���̾� ���̰����� ������ ���̰��� ��Ŭ ���.
		// ���� ������ ���̰��� ���� ���� ���̰�, ���� ���� ���̰� ������ ������ ���� ��Ȯ�� Offset Vector����
		// ����س���. // ������ �߿� 
		if (CurHeightLayer < CurSampledHeight )
		{
			float delta1 = (CurSampledHeight - CurHeightLayer);
			float delta2 = (CurHeightLayer + StepSize) - LastSampledHeight;
			float ratio = delta1 * rcp((delta1 + delta2));
			CurOffset = (ratio)* LastOffset + (1.0f - ratio) * CurOffset;
			CurSampleNum = SampleNum + 1;
		}
		else
		{
			CurSampleNum++;
			CurHeightLayer -= StepSize;
			CurOffset += StepSize * MaxOffset;
			LastOffset = CurOffset;
			LastSampledHeight = CurSampledHeight;
		}
	}

	float2 resultTex = pin.Tex + CurOffset;
	float4 resultNormal = g_HeightMap.Sample(samLinear, resultTex);
	float4 resultColor = g_DiffuseMap.Sample(samLinear, resultTex);
	float specIntensity = g_SpecularMap.Sample(samLinear, resultTex);

	// �� ������ Gbuffer�� �־�ξ�, Deferred Rendering�� Ȱ����.
	// Parameter.
	// GBuffer_OUT PackGbuffe(float3 BaseColor, float3 Normal, float SpecIntensity, float SpecPower, float ViewZ)
	return PackGbuffe(resultColor, resultNormal.xyz, specIntensity, g_SpecExp, 0);

}







