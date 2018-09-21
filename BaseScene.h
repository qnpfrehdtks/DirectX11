#pragma once
#include "DXUT.h"

class cSky;
class cShadowMap;

class cBaseScene
{
public:
	cBaseScene(ID3D11Device* device, ID3D11DeviceContext* dc, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv );
	virtual ~cBaseScene();

	virtual bool Init() = 0;
	virtual void OnResize() = 0;
	virtual void UpdateScene(float dt) = 0;
	virtual void DrawScene() = 0;


protected:

	ID3D11Device * m_pDevice;
	ID3D11DeviceContext * m_pDC;
	ID3D11RenderTargetView* m_pRTV;
	ID3D11DepthStencilView* m_pDSV;

	cSky*      m_Sky;
	cShadowMap* m_ShadowMap;



};

