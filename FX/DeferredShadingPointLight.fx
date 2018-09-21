
#include "LightHelper.fx"


Texture2D g_DepthMap;
Texture2D g_DiffuseSpecIntMap;
Texture2D g_NormalMap;
Texture2D g_SpecPowMap;


// ===========================================
//                 SamplerState
// ===========================================

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;

};
SamplerState samNormalDepth
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
	// ���, Ȯ�뿡�� ����, �Ӹ��� �� ���͸�.
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;

};

// ===========================================
//              Constant Buffer
// ===========================================
cbuffer cbBufferUnPack
{
	float4 g_ProjMatProperty;
	float4x4 g_InvVP;
	float4x4 g_InvView;
	float4x4 g_WorldViewProj;
}

cbuffer cbPerFrame
{
	float4x4 g_LightProjection;
	float4   g_FrustumCorners[4];
	
	sPointLight g_PtLight;

	float3 g_EyePosW;
	float pad;

};

float ConvertToLinearDepth( float Depth)
{
	return g_ProjMatProperty.w / (Depth - g_ProjMatProperty.z);
}

// ===========================================
//                  Struct
// ===========================================
struct VertexIn
{
	float3 PosL : POSITION;
	float3 ToFarPlaneIndex : NORMAL;
	float2 Tex : TEXCOORD;
};

struct VertexOut
{
	float3 ToFarPlane : TEXCOORD0;
};


// ========================================================
//                     Vertex Shader
// ========================================================
VertexOut main(VertexIn vin)
{
	VertexOut vout;

	//vout.Tex = vin.Tex;
	vout.ToFarPlane = g_FrustumCorners[vin.ToFarPlaneIndex.x].xyz;

	return vout;
}

// ========================================================
//                  Constant Hull Shader
// ========================================================

struct PatchTess
{
	float TessEdges[4] : SV_TessFactor;
	float TessInside[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS()
{
	PatchTess hout;

	float TessFactor = 18.0f;
	hout.TessEdges[0] = hout.TessEdges[1] = hout.TessEdges[2] = hout.TessEdges[3] = TessFactor;
	hout.TessInside[0] = hout.TessInside[1] = TessFactor;

	return hout;
}

// ========================================================
//              Controll Point Hull Shader
// ========================================================

// ������ ���� ���̴�, ���⼭�� �׳� ���
// �� �������� �Է����� �޴´�.
struct HullOut
{
	float3 HemiDir : POSITION;
	float3 ToFarPlane : TEXCOORD0;
};

static const float3 HemilDir[2] = {
	float3(1.0, 1.0,1.0),
	float3(-1.0, 1.0, -1.0)
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
HullOut HS(
	InputPatch <VertexOut, 4> p,
	uint i : SV_OutputControlPointID,
	uint patchId : SV_PrimitiveID)
{
	HullOut ho;

	ho.ToFarPlane = p[i].ToFarPlane;
	//ho.Tex = p[i].Tex;
	ho.HemiDir = HemilDir[patchId];

	return ho;
}

// �ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ�
// ========================================================
//                      tessellation...
// ========================================================
// �ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ�

// ========================================================
//                      Domain Shader
// ========================================================
// Domain Shader�� ���� ::
// �׼����̼� �� UV ��ǥ�� �̿��Ͽ�,  Controll Point �ڷḦ �����ؼ� ������ ���� ��ġ�� �ؽ�ó �ڷḦ �����ϴ°�,
// ���� ���̸��� ���̸� �����Ͽ� ���� ������ �����Ѵ�.

struct DomainOut
{
	float4 Position : SV_POSITION;
	float2 cpPos	: TEXCOORD0;
	float3 ToFarPlane : TEXCOORD1;
};

// ���� Shader�� �׼��� �Ű�����  UV ���� �Է����� �޴´�.
// ����, Constant Hull Shader �� ����, Tess ����� �Է¹ް�, 
// OutPatch ���� ������ Hull Shader�� ��� �������� �Է����� �޴´�.
[domain("quad")]
DomainOut DS(
	PatchTess tess, 
	float2 uv : SV_DomainLocation,
	const OutputPatch<HullOut, 4> quad)
{
	// Transform the UV's into clip-space
	float2 posClipSpace = uv.xy * 2.0 - 1.0;

	// Find the absulate maximum distance from the center
	float2 posClipSpaceAbs = abs(posClipSpace.xy);
	float maxLen = max(posClipSpaceAbs.x, posClipSpaceAbs.y);

	// Generate the final position in clip-space
	float3 normDir = normalize(float3(posClipSpace.xy, (maxLen - 1.0)) * quad[0].HemiDir);
	float4 posLS = float4(normDir.xyz, 1.0);

	// Transform all the way to projected space
	DomainOut Output;
	Output.Position = mul(posLS, g_LightProjection);
	// Store the clip space position
	Output.cpPos = Output.Position.xy / Output.Position.w;
	Output.ToFarPlane = (quad[0].ToFarPlane );
	return Output;

}
   // ======================================================
   //                    Pixel Shader
   // ======================================================
float4 PS(DomainOut pin,
	uniform bool isDepth = false, 
	uniform bool isDiffuse = false,
	uniform bool isNormal = false,
	uniform bool isSpecPow = false) : SV_TARGET
{
	float4 ambient = float4(0.0f,0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// �ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ�
	// ========================================================
	//                     UnPack Gbuffer               
	// ========================================================
	// �ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ�

    // ================================================
    //                       Depth
	// ================================================
	// ���⼭ ������ Depth ���� ProjZ / ProjW ���� ���� ���̰��� ��Ÿ����.
	// ���� :http://ozlael.egloos.com/3226675 // ���� ���� ��� ���� ���̸� ����س���,
	//  ���� ���� ���� �ش� ��ũ�� �ȼ� ��ǥ�� ���� ���� ��ǥ�� �˾Ƴ���.
	 float3 Depth = g_DepthMap.Load(float3(pin.Position.xy, 0.0f)).xyz;

	 float linearDepth = ConvertToLinearDepth(Depth.x);

	 // ================================================
	 //                Diffuse + SpecInt;
	 // ================================================
	 float4 DiffuseColorSpecInt = g_DiffuseSpecIntMap.Load(float3(pin.Position.xy, 0.0f));


	 // ================================================
	 //                     Normal
	 // ================================================
	 //float3 Normal = g_NormalMap.Load(float3(pin.Position.xy, 0.0f)).xyz;
	 // Normal = normalize(Normal * 2.0 - 1.0);
	 float3 Normal = g_NormalMap.Load(float3(pin.Position.xy, 0.0f)).xyz;
	 Normal.xy = Normal * 2.0 - 1.0;
	 Normal.z = sqrt(1 - dot(Normal.xy, Normal.xy));

	 Normal = normalize(Normal);
	  // ================================================
	  //                    Spec Power
	  // ================================================
	 float SpecPow = g_SpecPowMap.Load(float3(pin.Position.xy, 0.0f)).x;

	 // ================================================
	 //                    Ray Calcul
	 // ================================================

	 // RayV ���� View Space����, CamPos���� FarPlane ������ Ray
	 //float4 RayV;
	 //RayV.xy = pin.cpPos.xy * g_ProjMatProperty.xy * linearDepth;
	 //RayV.z =  linearDepth;
	 //RayV.w = 1.0f;

	 //float3 RayW = mul(RayV, g_InvView).xyz;

	 float3 ViewRay = (linearDepth / pin.ToFarPlane.z) * pin.ToFarPlane;
	
	 float3 WorldFarRay = mul(pin.cpPos, (float4x4)g_InvVP).xyz;


	 float3 toEyeW = g_EyePosW - WorldFarRay;
	 toEyeW = normalize(toEyeW);
	// g_PtLight.pos = RayW;
	 // ================================================= //
	 //                  Pt Light Calcul
	 // =================================================
	 // �Լ� �Ű����� 
	 /*void ComputePointLightDeferred(
	     sPointLight L,
	     float3 pos,
	     float4 diffuseColor,
	     float3 normal,
	     float3 toEye,
         float specPow,
	     float specInt,
	     out float4 ambient,
	     out float4 diffuse,
	     out float4 spec  } */

	 float4 A, D, S;

	 ComputePointLightDeferred(
		 g_PtLight,
		 WorldFarRay,
		 float4(DiffuseColorSpecInt.xyz, 1.0f),
		 Normal, 
		 toEyeW,
		 SpecPow,
		 DiffuseColorSpecInt.w,
		 A, D, S);

	 ambient += A;
	 diffuse += D ;
	 spec += S;
		 
	 float4 resultLight = (ambient + diffuse) + spec;
	 resultLight.w = 1.0f;

	 return resultLight;
	 //return float4(pin.ToFarPlane, 1.0f);
	//return float4(linearDepth, linearDepth, linearDepth,1);
}


technique11 DeferredPtLight
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 DeferredDepth
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}


technique11 DeferredDiffuse
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}


technique11 DeferredNormal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, main()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

