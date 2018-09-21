

// Sine Function Heat Haze Example
//https://tympanus.net/codrops/2016/05/03/animated-heat-distortion-effects-webgl/

#include "LightHelper.fx"


Texture2D g_TextureMap;
Texture2DArray g_NoiseMap;

cbuffer PerFrame
{
	float4x4 g_ViewProj;
	float3 g_EyePosW;
	float g_Time;
	float4 g_Size = float4(6,14,1,1);
};


cbuffer PerObj
{
	float2 g_Tex[4] =
	{
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};
};


struct VertexIn
{
	float3 PosL : POSITION;
	float4 Color :COLOR;
};

struct VertexOut
{
	float3 CenterW : POSITION;
	float4 Color :COLOR;
};

struct GeoOut
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float2 Tex2 : TEXCOORD1;
	/*float3 PosW : POSITION;
	float3 NormalW : NORMAL;
	float2 Tex : TEXCOORD;*/

};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;

};

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;

};


VertexOut main(VertexIn vin)
{
	VertexOut vout;
	vout.CenterW = vin.PosL;
	vout.Color = vin.Color;
	return vout;
}




[maxvertexcount(4)]
void GS(point VertexOut gin[1],
	uint primID : SV_PrimitiveID,
	inout TriangleStream<GeoOut> triStream)
{
	float3 up = float3(0, 1, 0);
	float3 toEyeDir = g_EyePosW - gin[0].CenterW;

//	toEyeDir.y = 0.0f;
	toEyeDir = normalize(toEyeDir);

	float3 right = cross(up, toEyeDir);
	right = normalize(right);
	up = cross(toEyeDir, right);

	float HalfX = g_Size.x * 0.5f;
	float HalfY = g_Size.y * 0.5f;

	// 3 =============== 1
	// |                 |
	// |                 |
	// |                 |
	// |                 |
	// |                 |
	// |                 |
	// 2 --------------- 0
	float4 Pos[4];
	Pos[0] = float4(gin[0].CenterW + HalfX * right - HalfY * up, 1.0f);
	Pos[1] = float4(gin[0].CenterW + HalfX * right + HalfY * up, 1.0f);
	Pos[2] = float4(gin[0].CenterW - HalfX * right - HalfY * up, 1.0f);
	Pos[3] = float4(gin[0].CenterW - HalfX * right + HalfY * up, 1.0f);

	
	//GeoOut
	//{
	////	float4 PosH : SV_POSITION;
	//float3 PosW : POSITION;
	//float3 NormalW : NORMAL;
	//float2 Tex : TEXCOORD;
	//uint PrimID : SV_PrimitiveID;
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		GeoOut gout;

		gout.PosH = mul(Pos[i], g_ViewProj);
		gout.PosH.z = 1.0f;

		float4 texCoord = gout.PosH * rcp( gout.PosH.w);
		texCoord.x = texCoord.x * 0.5f + 0.5f;
		texCoord.y = texCoord.y * -0.5f + 0.5f;
		gout.Tex = texCoord.xy;
		gout.Tex2 = g_Tex[i];

		triStream.Append(gout);
	}

}


float2 SineFunc(float2 coord)
{

	float2 newCoord = coord * 2.0f - 1.0f;
	newCoord.x = 1.0f - abs(newCoord.x);
	newCoord.y = 1.0f - abs(newCoord.y);



	float frequency = 20.0f;
	float ampli = 0.01;

	float distFactor = pow(newCoord.y,2.0f);

	float distortion  = (sin(coord.y * frequency + g_Time * 3.0f) * ampli) * distFactor;

	return float2(coord.x + distortion, coord.y);
}




float2 NoiseMap(float2 coord, float2 coord2)
{
	float riseFactor = 0.4f;
	float distFactor = 0.01f;
	float2 distCoord = coord;

	// 시간과 오르는 속도를 곱해서 왜곡 속도 조절
	distCoord.y -= g_Time * riseFactor;

	// Noise Map에서 Sample하여 Noise의 값을 가져옴.
	float4 distValue = g_NoiseMap.SampleLevel(samLinear, float3(distCoord, 0.0f), 0.0f);

	// [0,1] 에서 [ -1, 1]로 변화
	float2 distPosOffset = (distValue.xy * 2.0f) - 1.0f;

	// 왜곡 정도 조절
	distPosOffset *= distFactor;

	// 외곽으로 갈수록 왜곡 정도를 줄이기 위해 Tex coord에 따라 ㅇ왜곡 정도를 조절한다.
	float2 newCoord = coord2 * 2.0f - 1.0f;
	newCoord.x = pow(1.0f - abs(newCoord.x),3 );
	newCoord.y = pow(1.0f - abs(newCoord.y),3 );

	distPosOffset *= (newCoord);

	// 기존 좌표에 왜곡된 좌표의 offset을 더해 왜곡된 Texture 좌표를 반환한다.
	float2 distortedTextureCoordinate = coord + distPosOffset;
	return distortedTextureCoordinate;
}


float4 PS(GeoOut pin) : SV_Target
{
	float4 result = g_TextureMap.Sample(samLinear, NoiseMap(pin.Tex, pin.Tex2) );
	return  result;
}

technique11 HazeBasic
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetGeometryShader(CompileShader(gs_5_0, GS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));

	}

}

