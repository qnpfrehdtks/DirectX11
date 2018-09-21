
// =====================================================================
//               시차 차폐 매핑(Parallax Occulusion Mapping)
// =====================================================================
//https://www.gamedev.net/articles/programming/graphics/a-closer-look-at-parallax-occlusion-mapping-r3262/
// parallax-occlusion-mapping 참고. 
//http://sunandblackcat.com/tipFullView.php?l=eng&topicid=28
// 그림 예제 참고.


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
	// [0,1] 로 정규화.
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

	// Sample 할 (Detail) 수준을 정하는 공식. 눈과 노말값이 얼마나 일치하느냐에 따라 
	uint numSamples = (uint)lerp(g_MinSample, g_MaxSample, dot(fromEyeDirW, NormalW));

	// Bitangent 와 Tangent벡터를 이용하여 Tangent Space Transformation Matrix를 만들어 준다.
	float3 N = normalize(NormalW);
	float3 T = normalize(TangentW - dot(TangentW, N) * N);
	float3 B = cross(N, T);

	float3x3 tangentToWorldSpace;
	tangentToWorldSpace[0] = T;
	tangentToWorldSpace[1] = B;
	tangentToWorldSpace[2] = N;

	// 회전 행렬이므로, Transpose 함수를 통해 역행렬을 만들어 준다.
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

	// 최대 시차 오프셋의 크기 설정.
	float ParallaxLimit = -length( fromEyeT.xy) * rcp(fromEyeT.z);
	ParallaxLimit *= g_HeightScale;

	// Offset 계산을 위해서 FromEye vector의 방향을 정규하하여 사용하자.
	float2 OffsetDir = normalize(fromEyeT.xy);
	// maximum parallax offset를 오프셋의 스케일을 조정해준다.
	float2 MaxOffset = OffsetDir * ParallaxLimit;


	// Occulusion Mapping 을 위해서 높이를 같은 레이어의 크기만큼 나누고, 표면의 상단(1.0f) 부터 시작해서 0.0f까지 측정해나간다.
	// 0.0f 까지 내려가면서 단계별로 높이값을 낮출 필요가 있는데, 그 높이값 단계를 여기서 결정.
	//
	int SampleNum = pin.NumSamples;
	float StepSize = 1.0f *rcp( (float)SampleNum);


	// Texture 좌표 기울기를 수동으로 계산해내야 함.. 
	float2 dx = ddx(pin.Tex);
	float2 dy = ddy(pin.Tex);

	// 이제 EyeVector와 Height Map과 실제 교차하는 지점을 찾기위해 //
	// Parallax Occulsion 방법을 사용
	//만약 교차하는 지점을 찾으면 Loop 를 종료함.
	// 교차하지 않ㅇ면 EyeVector 방향으로 EyeLimit 값을 더하고, 그 더한 좌표를 HeightMap에서 추출하여 실제 높이를 찾아가야 함,
	// 교차가 일어날려면, 

	float CurHeightLayer = 1.0f; // step마다 진행할 높이값의 크기

	float CurSampledHeight = 1.0f; // 현재 단계에서 뽑아낸 높이값.
	float LastSampledHeight = 1.0f; // 저번 단계에서 뽑아낸 높이값,
	
	float2 CurOffset = float2(0, 0); // 현재 Eye Vector의 오프셋
	float2 LastOffset = float2(0, 0); // 지난 단계에서의 Eye Vector의 오프셋
	int CurSampleNum = 0; // 현재 Sample 진행한 수.
	 
	while (CurSampleNum < SampleNum) // Sample 만큼 진행할 것.
	{
		CurSampledHeight = g_HeightMap.SampleGrad(samLinear, pin.Tex + CurOffset, dx, dy);

		// 해당 레이어 높이값보다 추출한 높이값이 더클 경우.
		// 지금 추출한 높이값과 지난 샘플 높이값, 지금 샘플 높이값 사이의 보간을 통해 정확한 Offset Vector값을
		// 계산해낸다. // 보간이 중요 
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

	// 이 값들을 Gbuffer에 넣어두어, Deferred Rendering에 활용함.
	// Parameter.
	// GBuffer_OUT PackGbuffe(float3 BaseColor, float3 Normal, float SpecIntensity, float SpecPower, float ViewZ)
	return PackGbuffe(resultColor, resultNormal.xyz, specIntensity, g_SpecExp, 0);

}







