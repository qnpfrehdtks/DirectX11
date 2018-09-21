//-----------------------------------------------------------------------------------------
// Compute shader
//-----------------------------------------------------------------------------------------
Texture2D g_HDRTex;

// 1D ÀÔ·Â Æò±Õ Value
StructuredBuffer<float> g_AverageValues1D;
//StructuredBuffer<float> g_PrevAvgLum;

// Ãâ·Â Æò±Õ ÈÖµµ, ±¸Á¶Ã¼ ¹öÆÛ.,
RWStructuredBuffer<float> g_AverageLum;
RWTexture2D<float4> g_HDRDownScale;


cbuffer DownScaleConstants 
{
	uint g_ResX;
	uint g_ResY;
	uint g_Domain;
	uint g_GroupSize;
	float g_fAdaptation;
	float g_fBloomThreshold;
}

// Group shared memory to store the intermidiate results
groupshared float SharedPositions[1024];

static const float4 LUM_FACTOR = float4(0.299, 0.587, 0.114, 0);


[numthreads(1024, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID, uint3 groupThreadId : SV_GroupThreadID, uint3 groupId : SV_GroupID)
{
	uint2 CurPixel = uint2(dispatchThreadId.x % g_ResX, dispatchThreadId.x / g_ResX);
	//g_HDRDownScale[CurPixel.xy] = float4(1.0, 1.0, 1.0, 1.0);
	// Skip out of bound pixels
	float avgLum = 0.0;
	if (CurPixel.y < g_ResY)
	{
		int3 nFullResPos = int3(CurPixel * 4, 0);
		float4 downScaled = float4(0.0, 0.0, 0.0, 0.0);
		[unroll]
		for (int i = 0; i < 4; i++)
		{
			[unroll]
			for (int j = 0; j < 4; j++)
			{
				downScaled += g_HDRTex.Load(nFullResPos, int2(j, i));
			}
		}
		downScaled /= 16.0; // Average
		g_HDRDownScale[CurPixel.xy] = downScaled; // Store the qurter resulotion image
		avgLum = dot(downScaled, LUM_FACTOR); // Calculate the lumenace value for this pixel
	}
	SharedPositions[groupThreadId.x] = avgLum; // Store in the group memory for further reduction

	GroupMemoryBarrierWithGroupSync(); // Sync before next step

									   // Down scale from 1024 to 256
	if (groupThreadId.x % 4 == 0)
	{
		// Calculate the luminance sum for this step
		float stepAvgLum = avgLum;
		stepAvgLum += dispatchThreadId.x + 1 < g_Domain ? SharedPositions[groupThreadId.x + 1] : avgLum;
		stepAvgLum += dispatchThreadId.x + 2 < g_Domain ? SharedPositions[groupThreadId.x + 2] : avgLum;
		stepAvgLum += dispatchThreadId.x + 3 < g_Domain ? SharedPositions[groupThreadId.x + 3] : avgLum;

		// Store the results
		avgLum = stepAvgLum;
		SharedPositions[groupThreadId.x] = stepAvgLum;
	}

	GroupMemoryBarrierWithGroupSync(); // Sync before next step

									   // Downscale from 256 to 64
	if (groupThreadId.x % 16 == 0)
	{
		// Calculate the luminance sum for this step
		float stepAvgLum = avgLum;
		stepAvgLum += dispatchThreadId.x + 4 < g_Domain ? SharedPositions[groupThreadId.x + 4] : avgLum;
		stepAvgLum += dispatchThreadId.x + 8 < g_Domain ? SharedPositions[groupThreadId.x + 8] : avgLum;
		stepAvgLum += dispatchThreadId.x + 12 < g_Domain ? SharedPositions[groupThreadId.x + 12] : avgLum;

		// Store the results
		avgLum = stepAvgLum;
		SharedPositions[groupThreadId.x] = stepAvgLum;
	}

	GroupMemoryBarrierWithGroupSync(); // Sync before next step

									   // Downscale from 64 to 16
	if (groupThreadId.x % 64 == 0)
	{
		// Calculate the luminance sum for this step
		float stepAvgLum = avgLum;
		stepAvgLum += dispatchThreadId.x + 16 < g_Domain ? SharedPositions[groupThreadId.x + 16] : avgLum;
		stepAvgLum += dispatchThreadId.x + 32 < g_Domain ? SharedPositions[groupThreadId.x + 32] : avgLum;
		stepAvgLum += dispatchThreadId.x + 48 < g_Domain ? SharedPositions[groupThreadId.x + 48] : avgLum;

		// Store the results
		avgLum = stepAvgLum;
		SharedPositions[groupThreadId.x] = stepAvgLum;
	}

	GroupMemoryBarrierWithGroupSync(); // Sync before next step

									   // Downscale from 16 to 4
	if (groupThreadId.x % 256 == 0)
	{
		// Calculate the luminance sum for this step
		float stepAvgLum = avgLum;
		stepAvgLum += dispatchThreadId.x + 64 < g_Domain ? SharedPositions[groupThreadId.x + 64] : avgLum;
		stepAvgLum += dispatchThreadId.x + 128 < g_Domain ? SharedPositions[groupThreadId.x + 128] : avgLum;
		stepAvgLum += dispatchThreadId.x + 192 < g_Domain ? SharedPositions[groupThreadId.x + 192] : avgLum;

		// Store the results
		avgLum = stepAvgLum;
		SharedPositions[groupThreadId.x] = stepAvgLum;
	}

	GroupMemoryBarrierWithGroupSync(); // Sync before next step

									   // Downscale from 4 to 1
	if (groupThreadId.x == 0)
	{
		// Calculate the average lumenance for this thread group
		float fFinalAvgLum = avgLum;
		fFinalAvgLum += dispatchThreadId.x + 256 < g_Domain ? SharedPositions[groupThreadId.x + 256] : avgLum;
		fFinalAvgLum += dispatchThreadId.x + 512 < g_Domain ? SharedPositions[groupThreadId.x + 512] : avgLum;
		fFinalAvgLum += dispatchThreadId.x + 768 < g_Domain ? SharedPositions[groupThreadId.x + 768] : avgLum;
		fFinalAvgLum /= 1024.0;

		g_AverageLum[groupId.x] = fFinalAvgLum; // Write the final value into the 1D UAV which will be used on the next step
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Second pass - convert the 1D average values into a single value
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_GROUPS 64

// Group shared memory to store the intermidiate results
groupshared float SharedAvgFinal[MAX_GROUPS];

[numthreads(MAX_GROUPS, 1, 1)]
void DownScaleSecondPass(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID,
	uint3 dispatchThreadId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	// Fill the shared memory with the 1D values
	float avgLum = 0.0;
	if (dispatchThreadId.x < g_GroupSize)
	{
		avgLum = g_AverageValues1D[dispatchThreadId.x];
	}
	SharedAvgFinal[dispatchThreadId.x] = avgLum;

	GroupMemoryBarrierWithGroupSync(); // Sync before next step

									   // Downscale from 64 to 16
	if (dispatchThreadId.x % 4 == 0)
	{
		// Calculate the luminance sum for this step
		float stepAvgLum = avgLum;
		stepAvgLum += dispatchThreadId.x + 1 < g_GroupSize ? SharedAvgFinal[dispatchThreadId.x + 1] : avgLum;
		stepAvgLum += dispatchThreadId.x + 2 < g_GroupSize ? SharedAvgFinal[dispatchThreadId.x + 2] : avgLum;
		stepAvgLum += dispatchThreadId.x + 3 < g_GroupSize ? SharedAvgFinal[dispatchThreadId.x + 3] : avgLum;

		// Store the results
		avgLum = stepAvgLum;
		SharedAvgFinal[dispatchThreadId.x] = stepAvgLum;
	}

	GroupMemoryBarrierWithGroupSync(); // Sync before next step

									   // Downscale from 16 to 4
	if (dispatchThreadId.x % 16 == 0)
	{
		// Calculate the luminance sum for this step
		float stepAvgLum = avgLum;
		stepAvgLum += dispatchThreadId.x + 4 < g_GroupSize ? SharedAvgFinal[dispatchThreadId.x + 4] : avgLum;
		stepAvgLum += dispatchThreadId.x + 8 < g_GroupSize ? SharedAvgFinal[dispatchThreadId.x + 8] : avgLum;
		stepAvgLum += dispatchThreadId.x + 12 < g_GroupSize ? SharedAvgFinal[dispatchThreadId.x + 12] : avgLum;

		// Store the results
		avgLum = stepAvgLum;
		SharedAvgFinal[dispatchThreadId.x] = stepAvgLum;
	}

	GroupMemoryBarrierWithGroupSync(); // Sync before next step

									   // Downscale from 4 to 1
	if (dispatchThreadId.x == 0)
	{
		// Calculate the average luminace
		float fFinalLumValue = avgLum;
		fFinalLumValue += dispatchThreadId.x + 16 < g_GroupSize ? SharedAvgFinal[dispatchThreadId.x + 16] : avgLum;
		fFinalLumValue += dispatchThreadId.x + 32 < g_GroupSize ? SharedAvgFinal[dispatchThreadId.x + 32] : avgLum;
		fFinalLumValue += dispatchThreadId.x + 48 < g_GroupSize ? SharedAvgFinal[dispatchThreadId.x + 48] : avgLum;
		fFinalLumValue /= 64.0;

		// Store the final value
		g_AverageLum[0] = max(fFinalLumValue, 0.0001);

	}
	//g_AverageLum[0] = 1.0f;
}


//-----------------------------------------------------------------------------------------
// Bloom compute shader
//-----------------------------------------------------------------------------------------

Texture2D g_HDRDownScaleTex;
StructuredBuffer<float> g_InputAvgLum;
RWTexture2D<float4> g_OutBloom;

[numthreads(1024, 1, 1)]
void BloomReveal(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint2 CurPixel = uint2(dispatchThreadId.x % g_ResX, dispatchThreadId.x / g_ResX);

	// Skip out of bound pixels
	if (CurPixel.y < g_ResY)
	{
		float4 color = g_HDRDownScaleTex.Load(int3(CurPixel, 0));
		float Lum = dot(color, LUM_FACTOR);
		float avgLum = g_InputAvgLum[0];

		// Find the color scale
		float colorScale = saturate(Lum - avgLum * g_fBloomThreshold);

		g_OutBloom[CurPixel.xy] = color * colorScale;
	//	g_OutBloom[CurPixel.xy] = color;
	}
}



technique11 DownScaleFirst
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, main()));
	}
}

technique11 DownScaleSecond
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, DownScaleSecondPass()));
	}
}

technique11 BloomRevealFX
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, BloomReveal()));
	}
}

