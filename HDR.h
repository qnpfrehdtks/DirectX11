#pragma once
#include "DXUT.h"



class cHDR
{
private:

	ID3D11Device * m_pDevice;
	ID3D11DeviceContext* m_pDC;

	// 1D intermediate storage for the down scale operation
	ID3D11Buffer * m_pDownScale1DBuffer;
	ID3D11UnorderedAccessView* m_pDownScale1DUAV;
	ID3D11ShaderResourceView* m_pDownScale1DSRV;

	// Average luminance
	ID3D11Buffer* m_pAvgLumBuffer;
	ID3D11UnorderedAccessView* m_pAvgLumUAV;
	ID3D11ShaderResourceView* m_pAvgLumSRV;

	UINT m_Width;
	UINT m_Height;
	UINT m_DownScaleGroups;

	float m_fMiddleGray;
	float m_fLumWhiteSqr;

	UINT m_OffsetIBCount;
	UINT m_OffsetVBCount;

	ID3D11Buffer* m_VB;
	ID3D11Buffer* m_IB;

public:
	cHDR(ID3D11Device* device, ID3D11DeviceContext* dc, UINT width, UINT height);
	~cHDR();

	void Init();
	void Release();

	void ComputeDownScale(ID3D11ShaderResourceView* pHDRSRV);
	void ToneMapping(ID3D11ShaderResourceView* pHDRSR);
	void PostProcessing(ID3D11ShaderResourceView* pHDRSRV, ID3D11RenderTargetView* pLDRRTV);

	void BuildOffScreenOffSet();



};

