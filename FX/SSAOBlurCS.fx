// 
// Source�� ������ DirectX11 å ����.

// =========================================
//                  Texture
// =========================================
Texture2D g_NormalMap;
Texture2D g_DepthMap;
Texture2D g_InputMap;

// �ؽ�ó ��� ����.
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

// �ȼ��� �帱 �ݰ� ����,,,
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

// �׷� �����尡 ������ �޸��� ũ�⸦  ������ ũ��  + (2 * �帲�� �� ������ũ�� )
groupshared GroupM g_Cache[ChacheSize];

[numthreads(N,1,1)]
void main(
int3 groupThreadID : SV_GroupThreadID, 
int3 dispatchThreadID: SV_DispatchThreadID) {
	//float2 texCoord = dispatchThreadID.xy / float2(dispatchThreadID.Length.x, dispatchThreadID.Length.y);
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

		float2 TexCoord = CoordToTexCoord(x, dispatchThreadID.y);

		g_Cache[groupThreadID.x].Color = g_InputMap[int2(x, dispatchThreadID.y)];
		g_Cache[groupThreadID.x].Normal = g_NormalMap.SampleLevel( samLinear, TexCoord,0).xyz * 2.0f - 1.0f;
		g_Cache[groupThreadID.x].Depth = LinearDepth(g_DepthMap.SampleLevel(samLinear, TexCoord, 0).x);
	}
	if (groupThreadID.x >= N - g_BlurRadius)
	{
		// ������ ������ �������� ����ֱ� ���� ����
		int x = min(dispatchThreadID.x + g_BlurRadius, g_InputMap.Length.x - 1);

		float2 TexCoord = CoordToTexCoord(x, dispatchThreadID.y);

		g_Cache[groupThreadID.x + 2 * g_BlurRadius].Color = g_InputMap[int2(x, dispatchThreadID.y)];
		g_Cache[groupThreadID.x + 2 * g_BlurRadius].Normal = g_NormalMap.SampleLevel(samLinear, TexCoord, 0).xyz * 2.0f - 1.0f;
		g_Cache[groupThreadID.x + 2 * g_BlurRadius].Depth = LinearDepth(g_DepthMap.SampleLevel(samLinear, TexCoord, 0).x);
	}

	//	float2 coord = 0;
	// �̹��� ���� Texel�� �̹����� ��輱���� �����ϱ� ���� Chache�� ����.
	float2 coord = min(dispatchThreadID.xy, g_InputMap.Length.xy - 1);
	float2 TexCoord = CoordToTexCoord(coord.x, coord.y);
	g_Cache[groupThreadID.x + g_BlurRadius].Color = g_InputMap[coord];
	g_Cache[groupThreadID.x + g_BlurRadius].Normal = g_NormalMap.SampleLevel(samLinear, TexCoord, 0).xyz * 2 - 1.0f;
	g_Cache[groupThreadID.x + g_BlurRadius].Depth = LinearDepth(g_DepthMap.SampleLevel(samLinear, TexCoord, 0).x);
	//float d = 0.0f;
	// ��� �����尡 ������ ������ ��ٸ���.
	GroupMemoryBarrierWithGroupSync();

	// �帮�� ����.
	// Shared Memomry���� ������ �ڱ� �ڽ��� �ȼ� �� ������.
	float4 originalColor = g_Cache[groupThreadID.x + g_BlurRadius].Color;
	// �ǰ�� �帮�� �߽� �� ������.
	float4 resultColor = g_Weights[5] * originalColor;

	// 
	float3 centerNormal = g_Cache[groupThreadID.x + g_BlurRadius].Normal;
	float centerDepth = g_Cache[groupThreadID.x + g_BlurRadius].Depth;

	float totalWeight = g_Weights[5];

	[unroll]
	// Blur �� ������ŭ for���� ���� ����
	for (int i = -g_BlurRadius; i <= g_BlurRadius; i++) {
		// �� ��� �ڱ� �ڽ� �ȼ��̸� �����ɷ� ����
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
	// ���⼭ DispatchThreadID�� Thread ��������
	// groupThreadID �Է� �ڷḦ �������� Index ������ ����.


	// N ���� �ȼ��� �帮�� ���ؼ���  (N + 2 * �帮�� ������)�� �ҷ��;� �Ѵ�,
	//  ������ �ٱ� �� �ȼ��� �帮�� ���ؼ��� �� �ٱ��� ������ �ʿ�� �ϱ� ������ ������ ���� ��ŭ �� �̾Ƽ� 
	// �����ؾ� �Ѵ�.
	// �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� ��
	// �� �� ������ ��ŭ�� �ٱ� ������,

	// �׷��� �ٱ� �� ������, ��  ������ ���� ���� ���� index�� ���� 
	// ������ X ID�� �ѹ��� �ƴ� �ι��� �� �ؼ� ���� �ҷ��;� �Ѵ�. 
	if (groupThreadID.y < g_BlurRadius)
	{
		// ������ ������ �������� ����ֱ� ���� ����
		// gInput�� 0�� ���� �ι� ����?

		int y = max(dispatchThreadID.y - g_BlurRadius, 0);

		float2 TexCoord = CoordToTexCoord(dispatchThreadID.x, y);

		g_Cache[groupThreadID.y].Color = g_InputMap[int2(dispatchThreadID.x, y)];
		g_Cache[groupThreadID.y].Normal = g_NormalMap.SampleLevel(samLinear, TexCoord, 0).xyz * 2.0f - 1.0f;
		g_Cache[groupThreadID.y].Depth = LinearDepth(g_DepthMap.SampleLevel(samLinear, TexCoord, 0).x);
	}
	if (groupThreadID.y >= N - g_BlurRadius)
	{
		// ������ ������ �������� ����ֱ� ���� ����
		int y = min(dispatchThreadID.y + g_BlurRadius, g_InputMap.Length.y - 1);

		float2 TexCoord = CoordToTexCoord(dispatchThreadID.x, y);

		g_Cache[groupThreadID.y + 2 * g_BlurRadius].Color = g_InputMap[int2(dispatchThreadID.x, y)];
		g_Cache[groupThreadID.y + 2 * g_BlurRadius].Normal = g_NormalMap.SampleLevel(samLinear, TexCoord, 0).xyz * 2.0f - 1.0f;
		g_Cache[groupThreadID.y + 2 * g_BlurRadius].Depth = LinearDepth(g_DepthMap.SampleLevel(samLinear, TexCoord, 0).x);
	}

	//	float2 coord = 0;
	// �̹��� ���� Texel�� ���� ������ ����.
	float2 coord = min(dispatchThreadID.xy, g_InputMap.Length.xy - 1);

	float2 TexCoord = CoordToTexCoord(coord.x, coord.y);

	g_Cache[groupThreadID.y + g_BlurRadius].Color = g_InputMap[coord];
	g_Cache[groupThreadID.y + g_BlurRadius].Normal = g_NormalMap.SampleLevel(samLinear, TexCoord, 0).xyz * 2.0f - 1.0f;
	g_Cache[groupThreadID.y + g_BlurRadius].Depth = LinearDepth(g_DepthMap.SampleLevel(samLinear, TexCoord, 0).x);
	
	//float d = 0.0f;
	// ��� �����尡 ������ ������ ��ٸ���.
	GroupMemoryBarrierWithGroupSync();



	//// �帮�� ����.

	//float4 blurColor = float4(0, 0, 0, 0);

	//[unroll]
	//for (int i = -g_BlurRadius; i <= g_BlurRadius; i++)
	//{
	//	blurColor += g_Weights[i + g_BlurRadius] * g_Cache[groupThreadID.y + g_BlurRadius + i].Color;
	//}

	////	blurColor = g_Cache[groupThreadID.x];

	//g_OutputMap[dispatchThreadID.xy] = blurColor;
	// ============================================================= //
	// �帮�� ����.
	// �帮�� ����.
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


