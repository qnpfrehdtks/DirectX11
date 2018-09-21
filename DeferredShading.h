#pragma once
#include "DXUT.h"

using namespace DirectX;



class cCamera;
class cGBuffer;
class cShadowMap;


class DeferredShading
{

public:
	DeferredShading();
	~DeferredShading();

	void BuildFrustumCorners(float fov, float FarZ);
	void Init(ID3D11Device* dc, ID3D11DeviceContext* pDC);

	void LightPass(ID3D11DeviceContext* pDC, cGBuffer* Gbuffer, cCamera* cam, LPCSTR str, cShadowMap* shadowMap, ID3D11ShaderResourceView* ssaoMap);
	//void PointLightProcess(ID3D11DeviceContext* pDC, cGBuffer* Gbuffer, cCamera* cam, LPCSTR str);
	void BuildScreenQuad(ID3D11Device* pDevice);

	ID3D11Buffer* m_VB;
	ID3D11Buffer* m_IB;
	

private:
	XMFLOAT4 m_FrustumCorner[4];

	float A;
	float B;

};

