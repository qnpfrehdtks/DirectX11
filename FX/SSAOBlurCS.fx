// 
// Source나 원리는 DirectX11 책 참고.

// =========================================
//                  Texture
// =========================================
Texture2D g_NormalMap;
Texture2D g_DepthMap;
Texture2D g_InputMap;

// 텍스처 출력 변수.
RWTexture2D<float4> g_OutputMap;


static const float g_Weights[11] = {
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
	0.026995,
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;

};

cbuffer cbPerObj {
	float4 ProjAB;
};

// 픽셀을 흐릴 반경 길이,,,
cbuffer cbFixed
{
	static const int g_BlurRadius = 5;
};


// ========================================
//                  C  S
// ========================================
struct GroupM
{
	float4 Color;
	float3 Normal;
	float Depth;
};

float2 CoordToTexCoord(int x, int y) {
	return float2((float)x / (float)(g_InputMap.Length.x), (float)y / (float)g_InputMap.Length.y);
}

float LinearDepth(float depth) {
	return ProjAB.y / (depth - ProjAB.x);
}



#define N 256
#define ChacheSize ( N + 2 * g_BlurRadius)

// 그룹 스레드가 공유할 메모리의 크기를  스레드 크기  + (2 * 흐림을 할 반지름크기 )
groupshared GroupM g_Cache[ChacheSize];

[numthreads(N,1,1)]
void main(
int3 groupThreadID : SV_GroupThreadID, 
int3 dispatchThreadID: SV_DispatchThreadID) {
	//float2 texCoord = dispatchThreadID.xy / float2(dispatchThreadID.Length.x, dispatchThreadID.Length.y);
	// 여기서 DispatchThreadID는 Thread 영역으로
	// groupThreadID 입력 자료를 가져오는 Index 값으로 설정.


	// N 개의 픽셀을 흐리기 위해서는  (N + 2 * 흐리기 반지름)을 불러와야 한다,
	//  이유는 바깥 쪽 픽셀을 흐리기 위해서는 그 바깥의 공간을 필요로 하기 때문에 반지름 길이 만큼 더 뽑아서 
	// 저장해야 한다.
	// ■ ■ ■ ■ □ □ □ □ □ □ □ □ □ □ ■ ■ ■ ■
	// ■ 는 반지름 만큼의 바깥 스레드,

	// 그래서 바깥 쪽 스레드, 즉  반지름 길이 보다 적은 index를 가진 
	// 스레드 X ID는 한번이 아닌 두번을 더 텍셀 값을 불러와야 한다. 
	if (groupThreadID.x < g_BlurRadius)
{
		// 범위를 스레드 범위내로 잡아주기 위한 계산식
		// gInput의 0의 값을 두번 추출?

		int x = max(dispatchThreadID.x - g_BlurRadius, 0);

		float2 TexCoord = CoordToTexCoord(x, dispatchThreadID.y);

		g_Cache[groupThreadID.x].Color = g_InputMap[int2(x, dispatchThreadID.y)];
		g_Cache[groupThreadID.x].Normal = g_NormalMap.SampleLevel( samLinear, TexCoord,0).xyz * 2.0f - 1.0f;
		g_Cache[groupThreadID.x].Depth = LinearDepth(g_DepthMap.SampleLevel(samLinear, TexCoord, 0).x);
	}
	if (groupThreadID.x >= N - g_BlurRadius)
	{
		// 범위를 스레드 범위내로 잡아주기 위한 계산식
		int x = min(dispatchThreadID.x + g_BlurRadius, g_InputMap.Length.x - 1);

		float2 TexCoord = CoordToTexCoord(x, dispatchThreadID.y);

		g_Cache[groupThreadID.x + 2 * g_BlurRadius].Color = g_InputMap[int2(x, dispatchThreadID.y)];
		g_Cache[groupThreadID.x + 2 * g_BlurRadius].Normal = g_NormalMap.SampleLevel(samLinear, TexCoord, 0).xyz * 2.0f - 1.0f;
		g_Cache[groupThreadID.x + 2 * g_BlurRadius].Depth = LinearDepth(g_DepthMap.SampleLevel(samLinear, TexCoord, 0).x);
	}

	//	float2 coord = 0;
	// 이미지 밖의 Texel을 이미지의 경계선으로 설정하기 위해 Chache에 넣음.
	float2 coord = min(dispatchThreadID.xy, g_InputMap.Length.xy - 1);
	float2 TexCoord = CoordToTexCoord(coord.x, coord.y);
	g_Cache[groupThreadID.x + g_BlurRadius].Color = g_InputMap[coord];
	g_Cache[groupThreadID.x + g_BlurRadius].Normal = g_NormalMap.SampleLevel(samLinear, TexCoord, 0).xyz * 2 - 1.0f;
	g_Cache[groupThreadID.x + g_BlurRadius].Depth = LinearDepth(g_DepthMap.SampleLevel(samLinear, TexCoord, 0).x);
	//float d = 0.0f;
	// 모든 스레드가 연산을 끝내기 기다린다.
	GroupMemoryBarrierWithGroupSync();

	// 흐리기 시작.
	// Shared Memomry에서 저장한 자기 자신의 픽셀 값 가져옴.
	float4 originalColor = g_Cache[groupThreadID.x + g_BlurRadius].Color;
	// 맨가운데 흐리기 중심 값 가져옴.
	float4 resultColor = g_Weights[5] * originalColor;

	// 
	float3 centerNormal = g_Cache[groupThreadID.x + g_BlurRadius].Normal;
	float centerDepth = g_Cache[groupThreadID.x + g_BlurRadius].Depth;

	float totalWeight = g_Weights[5];

	[unroll]
	// Blur 할 영역만큼 for문을 돌며 실행
	for (int i = -g_BlurRadius; i <= g_BlurRadius; i++) {
		// 맨 가운데 자기 자신 픽셀이면 다음걸로 진행
		if (i == 0)  continue;

		float curID = groupThreadID.x + g_BlurRadius + i;

		float3 neighborNormal = g_Cache[curID].Normal;
		float neighborDepth = g_Cache[curID].Depth;


		if (dot(centerNormal, neighborNormal) >= 0.8f && 
			abs(neighborDepth - centerDepth) <= 0.1f) {

			float weight = g_Weights[g_BlurRadius + i];
			resultColor += (g_Cache[curID].Color * weight);
			totalWeight += weight;
		}
	}

	g_OutputMap[dispatchThreadID.xy] = resultColor / totalWeight;
}


[numthreads(1, N, 1)]
void VertBlurCS(
	int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID)
{
	
	//float2 texCoord = dispatchThreadID.xy / float2(dispatchThreadID.Length.x, dispatchThreadID.Length.y);
	// 여기서 DispatchThreadID는 Thread 영역으로
	// groupThreadID 입력 자료를 가져오는 Index 값으로 설정.


	// N 개의 픽셀을 흐리기 위해서는  (N + 2 * 흐리기 반지름)을 불러와야 한다,
	//  이유는 바깥 쪽 픽셀을 흐리기 위해서는 그 바깥의 공간을 필요로 하기 때문에 반지름 길이 만큼 더 뽑아서 
	// 저장해야 한다.
	// ■ ■ ■ ■ □ □ □ □ □ □ □ □ □ □ ■ ■ ■ ■
	// ■ 는 반지름 만큼의 바깥 스레드,

	// 그래서 바깥 쪽 스레드, 즉  반지름 길이 보다 적은 index를 가진 
	// 스레드 X ID는 한번이 아닌 두번을 더 텍셀 값을 불러와야 한다. 
	if (groupThreadID.y < g_BlurRadius)
	{
		// 범위를 스레드 범위내로 잡아주기 위한 계산식
		// gInput의 0의 값을 두번 추출?

		int y = max(dispatchThreadID.y - g_BlurRadius, 0);

		float2 TexCoord = CoordToTexCoord(dispatchThreadID.x, y);

		g_Cache[groupThreadID.y].Color = g_InputMap[int2(dispatchThreadID.x, y)];
		g_Cache[groupThreadID.y].Normal = g_NormalMap.SampleLevel(samLinear, TexCoord, 0).xyz * 2.0f - 1.0f;
		g_Cache[groupThreadID.y].Depth = LinearDepth(g_DepthMap.SampleLevel(samLinear, TexCoord, 0).x);
	}
	if (groupThreadID.y >= N - g_BlurRadius)
	{
		// 범위를 스레드 범위내로 잡아주기 위한 계산식
		int y = min(dispatchThreadID.y + g_BlurRadius, g_InputMap.Length.y - 1);

		float2 TexCoord = CoordToTexCoord(dispatchThreadID.x, y);

		g_Cache[groupThreadID.y + 2 * g_BlurRadius].Color = g_InputMap[int2(dispatchThreadID.x, y)];
		g_Cache[groupThreadID.y + 2 * g_BlurRadius].Normal = g_NormalMap.SampleLevel(samLinear, TexCoord, 0).xyz * 2.0f - 1.0f;
		g_Cache[groupThreadID.y + 2 * g_BlurRadius].Depth = LinearDepth(g_DepthMap.SampleLevel(samLinear, TexCoord, 0).x);
	}

	//	float2 coord = 0;
	// 이미지 밖의 Texel을 범위 안으로 조정.
	float2 coord = min(dispatchThreadID.xy, g_InputMap.Length.xy - 1);

	float2 TexCoord = CoordToTexCoord(coord.x, coord.y);

	g_Cache[groupThreadID.y + g_BlurRadius].Color = g_InputMap[coord];
	g_Cache[groupThreadID.y + g_BlurRadius].Normal = g_NormalMap.SampleLevel(samLinear, TexCoord, 0).xyz * 2.0f - 1.0f;
	g_Cache[groupThreadID.y + g_BlurRadius].Depth = LinearDepth(g_DepthMap.SampleLevel(samLinear, TexCoord, 0).x);
	
	//float d = 0.0f;
	// 모든 스레드가 연산을 끝내기 기다린다.
	GroupMemoryBarrierWithGroupSync();



	//// 흐리기 시작.

	//float4 blurColor = float4(0, 0, 0, 0);

	//[unroll]
	//for (int i = -g_BlurRadius; i <= g_BlurRadius; i++)
	//{
	//	blurColor += g_Weights[i + g_BlurRadius] * g_Cache[groupThreadID.y + g_BlurRadius + i].Color;
	//}

	////	blurColor = g_Cache[groupThreadID.x];

	//g_OutputMap[dispatchThreadID.xy] = blurColor;
	// ============================================================= //
	// 흐리기 시작.
	// 흐리기 시작.
	float4 originalColor = g_Cache[groupThreadID.y + g_BlurRadius].Color;
	float4 resultColor = g_Weights[5] * originalColor;

	float3 centerNormal = g_Cache[groupThreadID.y + g_BlurRadius].Normal;
	float centerDepth = g_Cache[groupThreadID.y + g_BlurRadius].Depth;

	float totalWeight = g_Weights[5];

	[unroll]
	/*for (int i = -g_BlurRadius; i <= g_BlurRadius; i++)
	{
	blurColor += g_Weights[i + g_BlurRadius] * g_Cache[groupThreadID.x + g_BlurRadius + i].Color;
	}*/

	for (int i = -g_BlurRadius; i <= g_BlurRadius; i++) {

		if (i == 0)  continue;

		float curID = groupThreadID.y + g_BlurRadius + i;

		float3 neighborNormal = g_Cache[curID].Normal;
		float neighborDepth = g_Cache[curID].Depth;

		if (dot(centerNormal, neighborNormal) >= 0.8f && 
			abs(neighborDepth - centerDepth) <= 0.1f) {

			float weight = g_Weights[g_BlurRadius + i];
			resultColor += (g_Cache[curID].Color * weight);
			totalWeight += weight;
		}
	}

	//	blurColor = g_Cache[groupThreadID.x];

	//g_OutputMap[dispatchThreadID.xy] = blurColor;

	g_OutputMap[dispatchThreadID.xy] = resultColor / totalWeight;
}



technique11 HorzBlur
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, main()));
	}
}

technique11 VertBlur
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, VertBlurCS()));
	}
}


