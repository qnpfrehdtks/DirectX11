#pragma once
#include "DXUT.h"


class cGBuffer
{
public:
	cGBuffer();
	~cGBuffer();

	HRESULT Init(ID3D11Device* device, UINT width, UINT height);

	void SwapToGbufferRTV(ID3D11DeviceContext* dc);
	void PostProcessing(ID3D11DeviceContext* dc);
	void PrepareForUnpack(ID3D11DeviceContext* dc);

	ID3D11Texture2D* GetColorTexture()              { return m_DiffuseSpecIntensityTex; }

	ID3D11DepthStencilView* GetDepthDSV()           { return m_DepthStencilDSV; }
	ID3D11DepthStencilView* GetDepthReadOnlyDSV()   { return m_DepthStencilReadOnlyDSV; }

	ID3D11ShaderResourceView* GetDepthSRV()         { return m_DepthStencilSRV; }
	ID3D11ShaderResourceView* GetDiffuseSRV()       { return m_DiffuseSpecIntensitySRV; }
	ID3D11ShaderResourceView* GetNormalSRV()        { return m_NormalSRV; }
	ID3D11ShaderResourceView* GetSpecPowerSRV()     { return m_SpecPowerSRV; }



private:
	ID3D11Device * m_pDevice;
	// GBuffer¿ë Texture
	ID3D11Texture2D* m_DepthStencilTex;
	ID3D11Texture2D* m_DiffuseSpecIntensityTex;
	ID3D11Texture2D* m_NormalTex;
	ID3D11Texture2D* m_SpecPowerTex;

	// GBuffer ·»´õÅ¸ÄÏ ºä
	ID3D11DepthStencilView* m_DepthStencilDSV;
	ID3D11DepthStencilView* m_DepthStencilReadOnlyDSV;

	ID3D11RenderTargetView* m_DepthRTV;
	ID3D11RenderTargetView* m_DiffuseSpecIntensityRTV;
	ID3D11RenderTargetView* m_NormalRTV;
	ID3D11RenderTargetView* m_SpecPowerRTV;

	// GBuffer SRV
	ID3D11ShaderResourceView* m_DepthStencilSRV;
	ID3D11ShaderResourceView* m_DiffuseSpecIntensitySRV;
	ID3D11ShaderResourceView* m_NormalSRV;
	ID3D11ShaderResourceView* m_SpecPowerSRV;


	ID3D11DepthStencilState *m_DepthStencilState;


};