#include "DXUT.h"
#include "LightMGR.h"
#include <memory>  
#include <iostream> 



void LightMGR::UpdateDirLights() 
{
}

LightMGR::LightMGR() : m_PtLightCount(0), m_DirLightCount(0)
{
	XMFLOAT4 up = XMFLOAT4(0, 1, 0, 0);
	m_UpVec = XMLoadFloat4(&up);
}

LightMGR::~LightMGR()
{

}

void LightMGR::Init()
{
	m_DirLights.reserve(4);
	m_PtLights.reserve(1000);

 //      sDirLight newDirLight = 
	//	CreateDirLight(
	//	XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
	//	XMFLOAT4(0.112f, 0.1f, 0.1f, 1.0f),
	//	XMFLOAT4(0.12f, 0.1f, 0.1f, 1.0f),
	//	XMFLOAT4(0.12f, 0.1f, 0.1f, 1.0f)
	//);

	float dir[3];
	dir[0] = -0.57735f;
	dir[1] = -0.57735f;
	dir[2] = 0.57735f;

   sDirLight newDirLight =
		   CreateDirLight(
			   XMFLOAT3(dir[0], dir[1], dir[2]),
			   XMFLOAT4(0.13f, 0.13f, 0.13f, 1.0f),
			   XMFLOAT4(0.13f, 0.13f, 0.13f, 1.0f),
			   XMFLOAT4(0.33f, 0.13f, 0.33f, 1.0f)
		   );


	m_DirLights.push_back(newDirLight);
}

void LightMGR::UpdateLightAngle(float x, float y, float z)
{


	XMFLOAT3 dir = XMFLOAT3(x, y, z);

	for (int i = 0; i < m_DirLightCount; i++) {
		m_DirLights[i].Direction = dir;

	}

}

void LightMGR::UpdateDirLightColor(float r, float g, float b)
{

	XMFLOAT4 color = XMFLOAT4(r, g, b, 0.0f);

	for (int i = 0; i < m_DirLightCount; i++) {
		m_DirLights[i].Ambient = color;
	
	}
}

void LightMGR::UpdateDirDiffuseLightColor(float r, float g, float b)
{
	XMFLOAT4 color = XMFLOAT4(r, g, b, 0.0f);

	for (int i = 0; i < m_DirLightCount; i++) {
		m_DirLights[i].Diffuse = color;

	}
}

void LightMGR::UpdateDirSpecLightColor(float r, float g, float b)
{
	XMFLOAT4 color = XMFLOAT4(r, g, b, 0.0f);

	for (int i = 0; i < m_DirLightCount; i++) {
		m_DirLights[i].Specular = color;

	}
}

void LightMGR::UpdatePointLight(float dt, float speed)
{
	for (int i = 0; i < m_PtLightCount - 4; i++)
	{
		XMMATRIX R = XMMatrixRotationAxis(m_UpVec, dt * speed);

		XMVECTOR lightPos = XMLoadFloat3(&m_PtLights[i].Position);
		lightPos = XMVector3TransformCoord(lightPos, R);
		XMStoreFloat3(&m_PtLights[i].Position, lightPos);
		XMStoreFloat3(&m_PtLightPos[i].Pos, lightPos);
	}



}

sPtLight LightMGR::CreatePointLight(
	XMFLOAT3& pos, 
	XMFLOAT4& ambient, 
	XMFLOAT4& diffuse, 
	XMFLOAT4& spec, 
	float Range, 
	XMFLOAT3& att)
{
	sPtLight newPtLight;
	Vertexs::sSimple ptPos;

	ptPos.Pos = pos;
	ptPos.Color = diffuse;

	newPtLight.Ambient = ambient;
	newPtLight.Diffuse = diffuse;
	newPtLight.Specular = spec;
	newPtLight.Att = att;
	newPtLight.Range = Range;
	newPtLight.Position = pos;

	m_PtLightPos.push_back(ptPos);
	m_PtLights.push_back(newPtLight);

	m_PtLightCount++;
	return newPtLight;
}

sDirLight LightMGR::CreateDirLight
(   XMFLOAT3& dir, 
	XMFLOAT4& ambient, 
	XMFLOAT4& diffuse, 
	XMFLOAT4& spec )
{
	sDirLight newDirLight;
	// 평행광 초기화.
	newDirLight.Ambient = ambient;
	newDirLight.Diffuse = diffuse;
	newDirLight.Specular = spec;
	newDirLight.Direction = dir;

	m_DirLights.push_back(newDirLight);

	m_DirLightCount++;

	return newDirLight;
}
