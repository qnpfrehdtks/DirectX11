#include "DXUT.h"
#include "BaseScene.h"


cBaseScene::cBaseScene(ID3D11Device* device, ID3D11DeviceContext * dc, ID3D11RenderTargetView * rtv, ID3D11DepthStencilView * dsv)
	: m_pDevice(device), m_pDC(dc), m_pDSV(dsv), m_pRTV(rtv)
{


}

cBaseScene::~cBaseScene()
{
	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pDC);
	SAFE_RELEASE(m_pDSV);
	SAFE_RELEASE(m_pRTV);


}


