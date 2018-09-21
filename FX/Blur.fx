
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


// �ȼ��� �帱 �ݰ� ����,,,
cbuffer cbFixed
{
	static const int g_BlurRadius = 5;
};

// �ؽ�ó �Է� ����
//Texture2D g_DepthMap;
Texture2D g_Input;
// �ؽ�ó ��� ����.
RWTexture2D<float4> g_Output;

#define N 256
#define ChacheSize ( N + 2 * g_BlurRadius)

// �׷� �����尡 ������ �޸��� ũ�⸦  ������ ũ��  + (2 * �帲�� �� ������ũ�� )
groupshared float4 g_Cache[ChacheSize];


[numthreads(N, 1, 1)]
void main(
	int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID)
{
	// ���⼭ DispatchThreadID�� Thread ��������
	// groupThreadID �Է� �ڷḦ �������� Index ������ ����.


	// N ���� �ȼ��� �帮�� ���ؼ���  (N + 2 * �帮�� ������)�� �ҷ��;� �Ѵ�,
	//  ������ �ٱ� �� �ȼ��� �帮�� ���ؼ��� �� �ٱ��� ������ �ʿ�� �ϱ� ������ ������ ���� ��ŭ �� �̾Ƽ� 
	// �����ؾ� �Ѵ�.
	// �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� ��
	// �� �� ������ ��ŭ�� �ٱ� ������,

	// �׷��� �ٱ� �� ������, ��  ������ ���� ���� ���� index�� ���� 
	// ������ X ID�� �ѹ��� �ƴ� �ι��� �� �ؼ� ���� �ҷ��;� �Ѵ�. 
	if (groupThreadID.x < g_BlurRadius)
	{
		// ������ ������ �������� ����ֱ� ���� ����
		// gInput�� 0�� ���� �ι� ����?

		int x = max(dispatchThreadID.x - g_BlurRadius, 0);
		g_Cache[groupThreadID.x] = g_Input[int2(x, dispatchThreadID.y)];
	}
	if (groupThreadID.x >= N - g_BlurRadius)
	{
		// ������ ������ �������� ����ֱ� ���� ����
		int x = min(dispatchThreadID.x + g_BlurRadius, g_Input.Length.x - 1);
		g_Cache[groupThreadID.x + 2 * g_BlurRadius] = g_Input[int2(x, dispatchThreadID.y)];
	}

	//	float2 coord = 0;
		// �̹��� ���� Texel�� ���� ������ ����.
	float2 coord = min(dispatchThreadID.xy, g_Input.Length.xy - 1);
	g_Cache[groupThreadID.x + g_BlurRadius] = g_Input[coord];
	//float d = 0.0f;
	// ��� �����尡 ������ ������ ��ٸ���.
	GroupMemoryBarrierWithGroupSync();

	// �帮�� ����.

	float4 blurColor = float4(0, 0, 0, 0);

		[unroll]
		for (int i = -g_BlurRadius; i <= g_BlurRadius; i++)
		{
			blurColor += g_Weights[i + g_BlurRadius] * g_Cache[groupThreadID.x + g_BlurRadius + i];
		}

	//	blurColor = g_Cache[groupThreadID.x];

	   g_Output[dispatchThreadID.xy] = blurColor;

}


// N ��ŭ y������ ������ ���� ����.
[numthreads(1, N, 1)]
void VertBlurCS(int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID)
{
	if (groupThreadID.y < g_BlurRadius)
	{
		// �������� ��迡 �°� �����.
		int y = max(dispatchThreadID.y - g_BlurRadius, 0);
		g_Cache[groupThreadID.y] = g_Input[int2(dispatchThreadID.x, y)];

	}
	if (groupThreadID.y >= N - g_BlurRadius)
	{
		// ������ ������ �������� ����ֱ� ���� ����
		int y = min(dispatchThreadID.y + g_BlurRadius, g_Input.Length.y - 1);
		g_Cache[groupThreadID.y + 2 * g_BlurRadius] = g_Input[int2(dispatchThreadID.x, y)];
	}

	//float2 coord = min(dispatchThreadID.xy, g_Input.Length.xy - 1);
	// �̹��� ���� Texel�� ���� ������ ����.

	//	float2 coord = 0;
	// �̹��� ���� Texel�� ���� ������ ����.
	float2 coord = min(dispatchThreadID.xy, g_Input.Length.xy - 1);
	g_Cache[groupThreadID.y + g_BlurRadius] = g_Input[coord];

	// ��� �����尡 ������ ������ ��ٸ���.
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
//// ��� ���̴����� �б� �������� ����� �ؽ�ó�� �����̴�.
//// ��� ���̴��� �Է����� ���Ǹ�, ���� �ؽ��Ϳ� ���� SRV�� ���� ���̴��� �����ش�.
//Texture2D gInputA;
//Texture2D gInputB;
//
//// �ؽ�ó ����� ��� ����.
//// ��� �ڿ��� Ư���ϰ� ���. ��� ���̴��� �� �ڿ��� ���ҵ��� �аų� �� �� ����.
//// Tamplate�� ���� ������ �����ؼ� ���. 
//// ��� ���̴����� �ڷḦ ����� �ڿ��� ���� ���ؼ���  ���� ���� ���� �䰡 �ʿ�.
//RWTexture2D <float4> gOutput;
//
//
// // ������ �׷��� ������ ũ�⸦ ���� , ���Ϳ� ���� x,y,z �� �����ϰ�, ���� 32������� 
//// �ϵ��� �����带 32���� ������ ���� ��ɾ ���� �����ϱ� ������
//// 32����� �����ϴ� ���� ����.
//[numthreads(16,16,1)]
//void CS(int3 displaypatchThreadID : SV_DispatchThreadID) // ������ ���̵�
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


