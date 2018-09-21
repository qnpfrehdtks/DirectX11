Texture2D<float4> g_Input;
RWTexture2D<float4> g_Output;

cbuffer cb0
{
	int ResX;
	int ResY;
}

static const float SampleWeights[13] = {
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
    0.008764,
    0.002216,
};

#define kernelhalf 6
#define groupthreads 128
groupshared float4 SharedInput[groupthreads];

[numthreads( groupthreads, 1, 1 )]
void main( 
	uint3 Gid : SV_GroupID, 
	uint GI : SV_GroupIndex )
{
	int X = Gid.x;
	int Y = GI - kernelhalf + (groupthreads - kernelhalf * 2) * Gid.y;

    int2 coord = int2( X, Y );
    coord = clamp( coord, int2(0, 0), int2(ResX -1, ResY -1) );
    SharedInput[GI] = g_Input.Load( int3(coord, 0) );  

    GroupMemoryBarrierWithGroupSync();

    // Vertical blur
    if ( GI >= kernelhalf && GI < (groupthreads - kernelhalf) && 
         ( (GI - kernelhalf + (groupthreads - kernelhalf * 2) * Gid.y) < ResY) )
    {
        float4 vOut = 0;
        
        [unroll]
        for ( int i = -kernelhalf; i <= kernelhalf; ++i )
		{
            vOut += SharedInput[GI + i] * SampleWeights[i + kernelhalf];
		}

		g_Output[coord] = float4(vOut.rgb, 1.0f);
    }
}

[numthreads( groupthreads, 1, 1 )]
void VertBlurCS( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex )
{
	int X = GI - kernelhalf + (groupthreads - kernelhalf * 2) * Gid.x;
	int Y = Gid.y;

    int2 coord = int2( GI - kernelhalf + (groupthreads - kernelhalf * 2) * Gid.x, Gid.y );
    coord = clamp( coord, int2(0, 0), int2(ResX -1, ResY -1) );
    SharedInput[GI] = g_Input.Load( int3(coord, 0) );        

    GroupMemoryBarrierWithGroupSync();

    // Horizontal blur
    if ( GI >= kernelhalf && GI < (groupthreads - kernelhalf) && 
         ( (Gid.x * (groupthreads - 2 * kernelhalf) + GI - kernelhalf) < ResX) )
    {
        float4 vOut = 0;
        
        [unroll]
        for ( int i = -kernelhalf; i <= kernelhalf; ++i )
            vOut += SharedInput[GI + i] * SampleWeights[i + kernelhalf];

		g_Output[coord] = float4(vOut.rgb, 1.0f);
    }
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