#pragma once

#include "DXUT.h"
#include "Gbuffer.h"
#include "Camera.h"

class cSSR
{

private:
	ID3D11RenderTargetView * m_ResultRTV;
	ID3D11UnorderedAccessView * m_ResultUAV;
	ID3D11ShaderResourceView* m_ResultSRV;

	ID3D11Buffer* m_VB;
	ID3D11Buffer* m_IB;

	UINT m_ThreadX;
	UINT m_ThreadY;

	ID3D11Texture2D* m_Tex;

public:
	cSSR();
	~cSSR();

	void Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDC, UINT width, UINT height);
	void BuildTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pDC);
	void BuildScreenQuad(ID3D11Device* pDevice);

	void DrawSSR(ID3D11DeviceContext* pDC, ID3D11ShaderResourceView* HDRSRV, cGBuffer* Gbuffer, cCamera* cam,  CXMMATRIX vp, float SSRFactor);
	void DrawSSR2(ID3D11DeviceContext* pDC, ID3D11DepthStencilView* dsv, ID3D11ShaderResourceView* HDRSRV, cGBuffer* Gbuffer, cCamera* cam, CXMMATRIX vp, float SSRFactor);
	ID3D11ShaderResourceView* GetResultSRV() { return m_ResultSRV; }



};

