
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;

};

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


// 픽셀을 흐릴 반경 길이,,,
cbuffer cbFixed
{
	static const int g_BlurRadius = 5;
};

// 텍스처 입력 변수
//Texture2D g_DepthMap;
Texture2D g_Input;
// 텍스처 출력 변수.
RWTexture2D<float4> g_Output;

#define N 256
#define ChacheSize ( N + 2 * g_BlurRadius)

// 그룹 스레드가 공유할 메모리의 크기를  스레드 크기  + (2 * 흐림을 할 반지름크기 )
groupshared float4 g_Cache[ChacheSize];


[numthreads(N, 1, 1)]
void main(
	int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID)
{
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
		g_Cache[groupThreadID.x] = g_Input[int2(x, dispatchThreadID.y)];
	}
	if (groupThreadID.x >= N - g_BlurRadius)
	{
		// 범위를 스레드 범위내로 잡아주기 위한 계산식
		int x = min(dispatchThreadID.x + g_BlurRadius, g_Input.Length.x - 1);
		g_Cache[groupThreadID.x + 2 * g_BlurRadius] = g_Input[int2(x, dispatchThreadID.y)];
	}

	//	float2 coord = 0;
		// 이미지 밖의 Texel을 범위 안으로 조정.
	float2 coord = min(dispatchThreadID.xy, g_Input.Length.xy - 1);
	g_Cache[groupThreadID.x + g_BlurRadius] = g_Input[coord];
	//float d = 0.0f;
	// 모든 스레드가 연산을 끝내기 기다린다.
	GroupMemoryBarrierWithGroupSync();

	// 흐리기 시작.

	float4 blurColor = float4(0, 0, 0, 0);

		[unroll]
		for (int i = -g_BlurRadius; i <= g_BlurRadius; i++)
		{
			blurColor += g_Weights[i + g_BlurRadius] * g_Cache[groupThreadID.x + g_BlurRadius + i];
		}

	//	blurColor = g_Cache[groupThreadID.x];

	   g_Output[dispatchThreadID.xy] = blurColor;

}


// N 만큼 y축으로 스레드 숫자 설정.
[numthreads(1, N, 1)]
void VertBlurCS(int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID)
{
	if (groupThreadID.y < g_BlurRadius)
	{
		// 스레드의 경계에 맞게 맞춘다.
		int y = max(dispatchThreadID.y - g_BlurRadius, 0);
		g_Cache[groupThreadID.y] = g_Input[int2(dispatchThreadID.x, y)];

	}
	if (groupThreadID.y >= N - g_BlurRadius)
	{
		// 범위를 스레드 범위내로 잡아주기 위한 계산식
		int y = min(dispatchThreadID.y + g_BlurRadius, g_Input.Length.y - 1);
		g_Cache[groupThreadID.y + 2 * g_BlurRadius] = g_Input[int2(dispatchThreadID.x, y)];
	}

	//float2 coord = min(dispatchThreadID.xy, g_Input.Length.xy - 1);
	// 이미지 밖의 Texel을 범위 안으로 조정.

	//	float2 coord = 0;
	// 이미지 밖의 Texel을 범위 안으로 조정.
	float2 coord = min(dispatchThreadID.xy, g_Input.Length.xy - 1);
	g_Cache[groupThreadID.y + g_BlurRadius] = g_Input[coord];

	// 모든 스레드가 연산을 끝내기 기다린다.
	GroupMemoryBarrierWithGroupSync();

	float4 blurColor = float4(0, 0, 0, 0);

	
		[unroll]
		for (int i = -g_BlurRadius; i <= g_BlurRadius; ++i)
		{
			blurColor += g_Weights[i + g_BlurRadius] * g_Cache[groupThreadID.y + g_BlurRadius + i];
		}
	
	//	blurColor = g_Cache[groupThreadID.y];

		g_Output[dispatchThreadID.xy] = blurColor;
	//g_Output[dispatchThreadID.xy] = blurColor;
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


//
//
//// 계산 쉐이더에서 읽기 전용으로 사용할 텍스처의 변수이다.
//// 계산 쉐이더의 입력으로 사용되며, 보통 텍스터와 같이 SRV를 통해 쉐이더에 묶어준다.
//Texture2D gInputA;
//Texture2D gInputB;
//
//// 텍스처 출력을 담는 예제.
//// 출력 자원은 특별하게 취급. 계산 쉐이더는 이 자원의 원소들을 읽거나 쓸 수 있음.
//// Tamplate로 따로 변수를 지정해서 사용. 
//// 계산 쉐이더에서 자료를 기록할 자원을 묶기 위해서는  순서 없는 접근 뷰가 필요.
//RWTexture2D <float4> gOutput;
//
//
// // 스레드 그룹의 스레드 크기를 설정 , 벡터와 같이 x,y,z 로 설정하고, 수는 32배수여야 
//// 하드웨어가 스레드를 32개의 단위로 묶어 명령어를 동시 수행하기 때문에
//// 32배수로 설정하는 것이 좋다.
//[numthreads(16,16,1)]
//void CS(int3 displaypatchThreadID : SV_DispatchThreadID) // 스레드 아이디
//{
//	gOutput[displaypatchThreadID.xy] =
//		gInputA[displaypatchThreadID.xy] +
//		gInputB[displaypatchThreadID.xy];
//}
//
//
//technique11 AddTexture
//{
//	pass P0
//	{
//		SetVertexShader(NULL);
//		SetPixelShader(NULL);
//		SetComputeShader(CompileShader(cs_5_0, CS()));
//	}
//}


