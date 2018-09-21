#pragma once
#include "DXUT.h"
#include <vector>


using namespace DirectX;

class cBasicShape;
class cCamera;


class cNormalDepthMap
{

private:
	XMFLOAT4 m_FrustumCorner[4];
	XMFLOAT4 m_Offset[14];

	std::vector<cBasicShape*> m_pShapeVector;

	UINT m_CountShape;
	UINT m_width;
	UINT m_height;

	ID3D11ShaderResourceView* m_NormalDepthSRV;
	ID3D11DepthStencilView* m_NormalDepthDSV;
	ID3D11RenderTargetView* m_NormalDepthRTV;

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;

public:
	cNormalDepthMap(ID3D11Device* device, ID3D11DeviceContext* dc, UINT width, UINT height);
	~cNormalDepthMap();

	// ==========================================
	//                    GET
	// ==========================================
	ID3D11ShaderResourceView* GetDepthSRV() { return m_NormalDepthSRV;  }
	ID3D11DepthStencilView* GetDepthDSV()   { return m_NormalDepthDSV; }
	ID3D11RenderTargetView* GetDepthRTV()   { return m_NormalDepthRTV; }

	XMFLOAT4* GetFrustumFarCorner() { return m_FrustumCorner; }
	

	void InsertShapeToDraw(cBasicShape* shape);
	void Init();
	void DrawToNormalDepth(cCamera* cam);
	void SwapToNormalMapDepthRTV(ID3D11DepthStencilView* dsv);

	void BuildFrustumFarCorners(float fovy, float farZ);
};

