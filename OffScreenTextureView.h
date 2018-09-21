#pragma once
#include "DXUT.h"



class cOffScreenTextureView
{
private:
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDC;

	ID3D11UnorderedAccessView * m_TexUAV;
	ID3D11ShaderResourceView* m_TexSRV;

	ID3D11ShaderResourceView* m_OffScreenSRV;
	ID3D11RenderTargetView* m_OffScreenRTV;

	ID3D11Buffer* m_OffScreenVB;
	ID3D11Buffer* m_OffScreenIB;

	UINT m_OffsetIBCount;
	UINT m_OffsetVBCount;

	UINT m_Width;
	UINT m_Height;


public:
	cOffScreenTextureView(ID3D11Device* device, ID3D11DeviceContext* dc, UINT width, UINT height);
	~cOffScreenTextureView();

	void Init();
	void BuildScreenGeometry();

	void SwapToOffScreen(ID3D11DepthStencilView* dsv);
	void DrawToScene();
	void DrawToScene(ID3D11ShaderResourceView* rsv);
	void BuildOffScreenView();


	ID3D11RenderTargetView* GetOffScreenRTV() { return m_OffScreenRTV; }
	ID3D11ShaderResourceView* GetOffSceenSRV() { return m_OffScreenSRV; }


};

