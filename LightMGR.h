//***************************************************************************************
// LightHelper.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Helper classes for lighting.
//***************************************************************************************

#ifndef LIGHTHELPER_H
#define LIGHTHELPER_H

#include <Windows.h>
#include <DirectXMath.h>
#include "singletonBase.h"
#include "Vertex.h"

using namespace DirectX;

// HLSL에서 자료 멤버들이 바이트 단위로 그냥 넣기 때문에,
// 두 개의 4차원 벡터에 걸쳐서 자료가 저장되는 경우가 발생한다.
// 그렇기 때문에 float pad 변수를 두어서, 두 벡터에 겹쳐서 저장되는 현상을 방지하자!!!

// 광원들의 여러 종류.
// 직접광(DirectionalLight), 점광(PointLight), 접점광(SpotLight)
// 정의.

namespace LightSource
{
	struct sDirLight
	{
		sDirLight() { ZeroMemory(this, sizeof(this)); }

		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular;
		XMFLOAT3 Direction;
		float Pad; // Pad the last float so we can set an array of lights if we wanted.
	};

	struct sPtLight
	{
		sPtLight() { ZeroMemory(this, sizeof(this)); }

		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular;
		XMFLOAT3 Position;

		float Range;
		XMFLOAT3 Att;
		float Pad; // Pad the last float so we can set an array of lights if we wanted.
	};


	struct sSpotLight
	{
		sSpotLight() { ZeroMemory(this, sizeof(this)); }

		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular;
		//
		XMFLOAT3 Position;
		float Range;
		//
		XMFLOAT3 Direction;
		float Spot;
		//
		XMFLOAT3 Att;
		float Pad; // Pad the last float so we can set an array of lights if we wanted.
	};
}

using namespace LightSource;

namespace Lights
{

	struct sDirectionalLight
	{
		sDirLight dirLight;
		XMFLOAT3 curAngle;
		XMFLOAT3 originAngle;
	};

	struct sPointLight
	{
		sPtLight ptLight;
		XMFLOAT3 curPos;
		XMFLOAT3 originPos;
	};
}


using namespace Lights;

struct sMaterial
{
	sMaterial() { ZeroMemory(this, sizeof(sMaterial)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	XMFLOAT4 Reflect;
};


class LightMGR : public SingletonBase<LightMGR>
{

private:
	XMVECTOR m_UpVec;
	XMFLOAT4 m_DirAxis;

	UINT m_DirLightCount;
	UINT m_PtLightCount;

	std::vector<sDirLight> m_DirLights;
	std::vector<sPtLight> m_PtLights;
	std::vector<Vertexs::sSimple> m_PtLightPos;
	//std::vector<sPtLight> m_PtLightsVestd::vector<sPtLight> m_PtLights;c;
	//std::vector<sSpotLight> m_SpotLights;

private:
	void UpdateDirLights();

public:

	// ====================================================
	//                          GET
	// ====================================================
	sDirLight GetDirLight(UINT idx) { return m_DirLights[idx]; }

	std::vector<sDirLight>* GetDirLights()      { return &m_DirLights; }
	std::vector<Vertexs::sSimple>* GetPtLightsPoss() { return &m_PtLightPos; }
	std::vector<sPtLight>* GetPtLights()        { return &m_PtLights; }

	//std::vector<sPointLight*> GetPtLightsVec()                     { return m_PtLightsVec; }

	UINT GetPointLightCount()                                      { return m_PtLightCount;  }
	UINT GetDirectLightCount()                                     { return m_DirLightCount; }

public:
	LightMGR();
	~LightMGR();

	void Init();
	void UpdateLightAngle(float x, float y, float z);
	void UpdateDirLightColor(float r, float g, float b);
	void UpdateDirDiffuseLightColor(float r, float g, float b);
	void UpdateDirSpecLightColor(float r, float g, float b);
	void UpdatePointLight(float dt, float speed);

	sPtLight CreatePointLight(XMFLOAT3& pos , XMFLOAT4& ambient, XMFLOAT4& diffuse, XMFLOAT4& spec, float Range, XMFLOAT3& att);
	sDirLight CreateDirLight(XMFLOAT3& dir, XMFLOAT4& ambient, XMFLOAT4& diffuse, XMFLOAT4& spec);
};







#endif // LIGHTHELPER_H