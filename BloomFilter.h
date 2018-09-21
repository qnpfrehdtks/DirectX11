#pragma once
#include "DXUT.h"
#include "BlurFilter.h"

// ========================================
//       MS HDR Bloom example ยüฐํวิ.
// ========================================

class cBloomFilter
{
private:

	ID3D11Device * m_pDevice;
	ID3D11DeviceContext* m_pDC;


	// Downscaled scene texture
	ID3D11Texture2D* m_pDownScaleRT;
	ID3D11ShaderResourceView* m_pDownScaleSRV;
	ID3D11UnorderedAccessView* m_pDownScaleUAV;

	// Temporary texture
	ID3D11Texture2D* m_pTempTex[2];
	ID3D11RenderTargetView* m_pTempRT[2];
	ID3D11ShaderResourceView* m_pTempSRV[2];
	ID3D11UnorderedAccessView* m_pTempUAV[2];

	// Bloom texture
	ID3D11Texture2D* m_pBloomRT;
	ID3D11ShaderResourceView* m_pBloomSRV;
	ID3D11UnorderedAccessView* m_pBloomUAV;


	// 1D intermediate storage for the down scale operation
	ID3D11Buffer * m_pDownScale1DBuffer;
	ID3D11UnorderedAccessView* m_pDownScale1DUAV;
	ID3D11ShaderResourceView* m_pDownScale1DSRV;

	// Average luminance
	ID3D11Buffer* m_pAvgLumBuffer;
	ID3D11UnorderedAccessView* m_pAvgLumUAV;
	ID3D11ShaderResourceView* m_pAvgLumSRV;


	// Result
	ID3D11ShaderResourceView* m_pResultSRV;
	ID3D11RenderTargetView* m_pResultRTV;

	UINT m_Width;
	UINT m_Height;
	UINT m_DownScaleGroups;

	float m_fMiddleGray;
	float m_fLumWhiteSqr;
	float m_fBloomThreshold;
	float m_fBloomScale;

	UINT m_OffsetIBCount;
	UINT m_OffsetVBCount;

	ID3D11Buffer* m_VB;
	ID3D11Buffer* m_IB;

public:
	cBloomFilter(ID3D11Device* device, ID3D11DeviceContext* dc, UINT width, UINT height);
	~cBloomFilter();

	void Init();
	void Release();

	void ComputeDownScale(ID3D11ShaderResourceView* pHDRSRV);
	void ToneMapping(ID3D11ShaderResourceView* pHDRSR);
	void PostProcessing(ID3D11ShaderResourceView* pHDRSRV, ID3D11RenderTargetView* pLDRRTV, ID3D11DepthStencilView* pDSV = nullptr);

	void BuildOffScreenOffSet();
	void BuildResultTexture();

	void Blur(ID3D11ShaderResourceView* pInput, ID3D11UnorderedAccessView* pOutput, UINT blurCount);
	void Bloom();

	ID3D11ShaderResourceView* GetBloomSRV() { return m_pBloomSRV; }
	ID3D11ShaderResourceView* GetResultSRV() { return m_pResultSRV; }



};

