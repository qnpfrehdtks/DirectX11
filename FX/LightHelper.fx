

struct sDirectLight
{
	float4 ambient;
	float4 diffuse;
	float4 spec;
	float3 dir;
	float pad;
};


struct sPointLight
{
	float4 ambient;
	float4 diffuse;
	float4 spec;

	float3 pos;
	float range;
	float3 att;
	float pad;
};


struct sSpotLight
{
	float4 ambient;
	float4 diffuse;
	float4 spec;

	float3 pos;
	float range;

	float3 dir;
	float spot;
	float3 att;
	float pad;
};

struct sMaterial
{
	float4 ambient;
	float4 diffuse;
	float4 spec;
	float4 reflect;
};
void ComputeDirectionLight(
	sMaterial mat, sDirectLight L,
	float3 normal, float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = -L.dir;

	ambient = mat.ambient * L.ambient;
	float diffuseFactor = dot(normal, lightVec);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		// �ݻ�Ǵ� ���� ����
		float3 reflectedVec = reflect(-lightVec, normal);

		float specFactor = pow(max(dot(reflectedVec, toEye), 0.0f), mat.spec.w);
		diffuse = diffuseFactor * mat.diffuse * L.diffuse;
		spec = specFactor * mat.spec * L.spec;
	}

}


void ComputeDirectionLightDeferred(
	sDirectLight L,
	float4 diffuseColor,
	float3 normal,
	float3 toEye,
	float specPow,
	float specInt,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec
	)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);


	float3 toLightVec = -L.dir;

	ambient = L.ambient * diffuseColor;
	float diffuseFactor = dot(normal, toLightVec);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		
		// �ݻ�Ǵ� ���� ����
	
		float3 r = reflect(-toLightVec, normal);
		float RDotV = dot(r, toEye);
		float specFactor = pow(max(RDotV, 0.0f), specPow);

		diffuse = diffuseFactor * diffuseColor * L.diffuse;
		spec = specFactor * specInt * L.spec;

	}

}





void ComputeToonDirectionLight(
	sMaterial mat, sDirectLight L,
	float3 normal, float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = -L.dir;

	ambient = mat.ambient * L.ambient;
	float diffuseFactor = dot(normal, lightVec);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		// �ݻ�Ǵ� ���� ����
		float3 reflectedVec = reflect(-lightVec, normal);

		float specFactor = pow(max(dot(reflectedVec, toEye), 0.0f), mat.spec.w);

		////// ī�� �������� ���� ��.
		////// �������� ���� �ƴ� ������ ������ �� ������.
		if (specFactor <= 0.1f)
		{
			specFactor = 0.1;
		}
		else if (specFactor > 0.1f && specFactor <= 0.8f)
		{
			specFactor = 0.35f;
		}
		else if (specFactor > 0.8f && specFactor <= 1.0f)
		{
			specFactor = 0.8f;
		}

		if (diffuseFactor <= 0.0f)
		{
			diffuseFactor = 0.1f;
		}
		else if (diffuseFactor > 0.0f && diffuseFactor <= 0.5f)
		{
			diffuseFactor = 0.35f;
		}
		else if (diffuseFactor > 0.5f && diffuseFactor <= 1.0f)
		{
			diffuseFactor = 0.8f;
		}


		diffuse = diffuseFactor * mat.diffuse * L.diffuse;
		spec = specFactor * mat.spec * L.spec;
	}

}


void ComputePointLightDeferred(
	sPointLight L,
	float3 pos,
	float4 diffuseColor,
	float3 normal,
	float3 toEye,
	float specPow,
	float specInt,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec
)
{
	ambient = float4(0, 0, 0, 0);
	diffuse = float4(0, 0, 0, 0);
	spec = float4(0, 0, 0, 0);


	float3 toLightVec = L.pos - pos;
	float d = length(toLightVec);
	

	if (d >= L.range) return;

	ambient = L.ambient * diffuseColor;

	float i = 0.1f;
	float3 AttVec = (2.0f)*(toLightVec * rcp(L.range));
	
	float outerRadius = L.range;
	float innerRadius = L.range / 3.0f;

	toLightVec = normalize(toLightVec);

	//ambient = L.ambient * diffuseColor;
	float diffuseFactor = dot(normal, toLightVec);
	

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		
		diffuse = diffuseFactor * L.diffuse * diffuseColor;

		// Phong specular
		//float3 h = normalize((toEye + toLightVec) * 0.5f);
		float3 r = reflect(-toLightVec, normal);
		float RDotV = dot(r, toEye);
		float specFactor = pow(max(RDotV, 0.0f), specPow);

		spec = specFactor * specInt * L.spec;

	
	}
	
	float att = 1.0f * rcp( dot(L.att, float3(1.0f, d, d*d)));
	att *=  (( saturate( (outerRadius - d) / (outerRadius - innerRadius))));
	//float att = i * exp( -(dot(AttVec, AttVec)));

    att = pow(att, 2.0f);
		
	ambient *= att;
	diffuse *= att;
	spec *= att;
	
}



// Normal Mapping�� ���� �Լ��� ����
// �� ���� ��(2D 2���� �ؽ�ó�ε�, Normal ���� 24bit �� ������ ����.)���� ������ Normal��  (���⼭ ������ ������ [0, 1] ��)
//  [-1,1]�� Mapping �Ͽ� Normal���� �����´�.
float3 NormalSampleToWorldSpace(
	float3 normalMapSample, // Normal Map���� ������ ����,
	float3 unitNormalW,     // World ��ǥ���� �� Normal Vector
	float3 tangentW)       // World ��ǥ�� �� Tangent Vector
{
	// Sample �Լ����� ������ ���� ���е��� [0,1] ���� [-1,1]�� ��ȯ��Ų��.
	float3 normalT = 2.0f * normalMapSample - 1.0f;

	// World �� ���� ����  World ������ T B N Basis Vector�� ���ϰ�, ��ȯ ��ķ� ����� ������.
	// �׶�-����Ʈ �� Tangent ��� Normal ���� ��������ȭ ������ ��ģ��.
	// ����  T - Proj n(T) 
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N) * N);

	float3 B = cross(N, T);

	// World�� ���� ���� Matrix;
	float3x3 worldTransformMat = float3x3(T, B, N);

	float3 bumpedNormal = mul(normalT, worldTransformMat);

	return bumpedNormal;


}

static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalcShadowFactorWithPCF(SamplerComparisonState samShadow,
	Texture2D shadowMap,
	float4 shadowPosH)
{
	shadowPosH.xyz /= shadowPosH.w;
	// NDC ������ z��ǥ�� ���̷�
	float depth = shadowPosH.z;
	const float dx = SMAP_DX;

	// Shadow Map ũ�⿡ �´� �ȼ� Offset�� �غ���.
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	// ���ȭ�� ������ ����
	float percentLit = 0.0f;

	// 9���� Pixel �� ������.
	[unroll]
	for (int i = 0; i < 9; ++i) {
		//
		percentLit += shadowMap.SampleCmpLevelZero(samShadow,
			shadowPosH.xy + offsets[i], depth).r;
	}

	return percentLit / 9.0f;

}

float CalcShadowFactorWithPCF2X2(SamplerComparisonState samShadow,
	Texture2D shadowMap,
	float4 shadowPosH)
{
	// Complete projection by doing division by w.
	shadowPosH.xyz /= shadowPosH.w;

	// Depth in NDC space.
	float depth = shadowPosH.z;
	const float dx = SMAP_DX;

	float percentLit = 0.0f;


		percentLit += shadowMap.SampleCmpLevelZero(samShadow,
			shadowPosH.xy , depth).r;

		return percentLit;

}


float CalcShadowFactorWithOutPCF(SamplerState samPoint,
	Texture2D shadowMap,
	float4 shadowPosH)
{
	shadowPosH.xyz /= shadowPosH.w;


	float resultShadow = 0.0f;

	resultShadow = shadowMap.Sample(samPoint,
		shadowPosH.xy).r;

	return resultShadow;
}





void ComputePointLight(
	sMaterial mat, sPointLight L,
	float3 pos,
	float3 normal, float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
	ambient = float4(0, 0, 0, 0);
	diffuse = float4(0, 0, 0, 0);
	spec = float4(0, 0, 0, 0);

	float3 toLight = L.pos - pos;
	float d = length(toLight);

	//if (d > L.range) return;

	ambient = L.ambient * mat.ambient;

	
	// Phong diffuse
	toLight /= d; // Normalize
	float diffuseFactor = dot(toLight, normal); // Normal �� �Ի簢�� ���� Lambert ���� Ȱ��
	diffuse = diffuseFactor * L.diffuse * mat.diffuse;

	// Phong specular
	//float3 halfWay = normalize(toEye + toLight);

	float3 reflectedLight = reflect(-toLight, normal);
	float specFactor = pow(max(dot(toEye, reflectedLight), 0.0f), mat.spec.w);
	spec = specFactor * mat.spec * L.spec;

	// Attenuation
	float att = 1.0f / ((d / L.range) + 1.0f);
	att = pow( att, 3.0f);

//	float att = 1.0f / dot(L.att, float3(1.0f, d, d*d));

	ambient *= att;
	diffuse *= att;
	spec *= att;

}

// =================================================
//                    Rim Light
// =================================================
float4 RimLightCalcul(float3 toEye, float3 Normal, float4 RimLightColor, float Width)
{
	return RimLightColor * smoothstep(1.0f - Width, 1.0f, 1.0f - max(0, dot(Normal, toEye)));
}

// =================================================
//                    Rim Light Pow
// =================================================
float4 RimLightCalcul2(float3 toEye, float3 Normal, float4 RimLightColor, float power)
{

	float rim = 1.0f - dot(Normal, toEye);
	float4 rimLighting = RimLightColor * pow(rim, power);
	  
	return rimLighting;
	
}
