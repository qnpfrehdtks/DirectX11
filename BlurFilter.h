#pragma once
#include "EffectMGR.h"
#include "Main.h"
#include "NormalDepthMap.h"

class cBlurFilter
{
private:
	
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDC;

	ID3D11ShaderResourceView * m_BlurOutputTexSRV;
	ID3D11UnorderedAccessView* m_BlurOutputTexUAV;

	ID3D11ShaderResourceView*  m_OffScreenSRV;
	ID3D11RenderTargetView*    m_OffScreenRTV;
	ID3D11UnorderedAccessView* m_OffScreenUAV;

	ID3D11ShaderResourceView*  m_BlurOffScreenSRV;
	ID3D11RenderTargetView*    m_BlurOffScreenRTV;

	ID3D11Buffer* m_OffScreenVB;
	ID3D11Buffer* m_OffScreenIB;

	UINT m_OffsetIBCount;
	UINT m_OffsetVBCount;

	UINT m_Width;
	UINT m_Height;

	DXGI_FORMAT m_Format;


public:
	cBlurFilter(ID3D11Device* device, ID3D11DeviceContext* dc, UINT width, UINT height);
	~cBlurFilter();

	void Init();
	void ComputeBlur(UINT blurCount,cNormalDepthMap* dmap);

	void BuildOffScreenGeometry();
	void BuildOffScreenViews();

	void SwapRTV(ID3D11DepthStencilView* dsv);
	void DrawScreen();

	ID3D11ShaderResourceView* GetBlurredOutputSRV()   { return m_BlurOutputTexSRV; }
	ID3D11UnorderedAccessView* GetBlurredOutputUAV()   { return m_BlurOutputTexUAV; }

	ID3D11RenderTargetView* GetOffScreenRTV()      { return m_OffScreenRTV; }
	ID3D11UnorderedAccessView* GetOffSceenUAV()    { return m_OffScreenUAV; }
	ID3D11ShaderResourceView* GetOffSceenSRV()     { return m_OffScreenSRV; }

};

