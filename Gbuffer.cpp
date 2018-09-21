#include "DXUT.h"
#include "Gbuffer.h"
#include <DirectXTex.h>
#include "Vertex.h"
#include "LightMGR.h"
#include "d3dx11effect.h"
#include "EffectMGR.h"
#include "RenderState.h"

cGBuffer::cGBuffer() : 
	m_DepthStencilDSV(nullptr), 
	m_DepthStencilReadOnlyDSV(nullptr), 
	m_DepthStencilSRV(nullptr),
	m_DepthStencilState(nullptr), 
	m_DepthStencilTex(nullptr)
{
}

cGBuffer::~cGBuffer()
{
}

HRESULT cGBuffer::Init(ID3D11Device* device, UINT width, UINT height  )
{

	m_pDevice = device;

	D3D11_TEXTURE2D_DESC tbd;
	tbd.Width = width;
	tbd.Height = height;
	tbd.MipLevels = 1;
	tbd.ArraySize = 1;
	tbd.Format = DXGI_FORMAT_UNKNOWN;
	tbd.SampleDesc.Count = 1;
	tbd.SampleDesc.Quality = 0;
	tbd.Usage = D3D11_USAGE_DEFAULT;

	tbd.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	tbd.CPUAccessFlags = 0;
	tbd.MiscFlags = 0;

	tbd.Format = DXGI_FORMAT_R24G8_TYPELESS;
	// 깊이 스텐실을 담는 View 부터.
	HR(m_pDevice->CreateTexture2D(&tbd, NULL, &m_DepthStencilTex));

	// =============== Diffuse + Spec Intensity ===================
	// Diffuse RGB 8bit + 8bit + 8bit, Spec Intensity 8bit 
	tbd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tbd.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	HR(m_pDevice->CreateTexture2D(&tbd, 0, &m_DiffuseSpecIntensityTex));

	// =============== Normal ===================
	tbd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	HR(m_pDevice->CreateTexture2D(&tbd, 0, &m_NormalTex));

	// =============== Spec Power ===================
	tbd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	HR(m_pDevice->CreateTexture2D(&tbd, 0, &m_SpecPowerTex));




	// ============================================
	//                 DSV 생성. 
	// ============================================
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd =
	{
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		0
	};

	HR(m_pDevice->CreateDepthStencilView(m_DepthStencilTex, &dsvd, &m_DepthStencilDSV));

	dsvd.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;
	HR(m_pDevice->CreateDepthStencilView(m_DepthStencilTex, &dsvd, &m_DepthStencilReadOnlyDSV));

	// ============================================
	//                 RTV 생성. 
	// ============================================

	D3D11_RENDER_TARGET_VIEW_DESC rtsvd =
	{
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D11_RTV_DIMENSION_TEXTURE2D
	};

	HR(m_pDevice->CreateRenderTargetView(m_DiffuseSpecIntensityTex, &rtsvd, &m_DiffuseSpecIntensityRTV));

	rtsvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	HR(m_pDevice->CreateRenderTargetView(m_NormalTex, &rtsvd, &m_NormalRTV));

	rtsvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	HR(m_pDevice->CreateRenderTargetView(m_SpecPowerTex, &rtsvd, &m_SpecPowerRTV));

	// ============================================
	//                 SRV 생성. 
	// ============================================
	// 쉐이더 자원 뷰로 생성하기 위해 구조체 서술.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd =
	{
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};

	srvd.Texture2D.MipLevels = 1;
	// Shader 자원뷰 생성.
	HR(m_pDevice->CreateShaderResourceView(m_DepthStencilTex, &srvd, &m_DepthStencilSRV));

	srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	HR(m_pDevice->CreateShaderResourceView(m_DiffuseSpecIntensityTex, &srvd, &m_DiffuseSpecIntensitySRV));

	srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	HR(m_pDevice->CreateShaderResourceView(m_NormalTex, &srvd, &m_NormalSRV));

	srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	HR(m_pDevice->CreateShaderResourceView(m_SpecPowerTex, &srvd, &m_SpecPowerSRV));

	D3D11_DEPTH_STENCIL_DESC descDepth;
	descDepth.DepthEnable = TRUE;
	descDepth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	descDepth.DepthFunc = D3D11_COMPARISON_LESS;
	descDepth.StencilEnable = TRUE;
	descDepth.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	descDepth.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp = { D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_COMPARISON_ALWAYS };
	descDepth.FrontFace = stencilMarkOp;
	descDepth.BackFace = stencilMarkOp;
	HR(m_pDevice->CreateDepthStencilState(&descDepth, &m_DepthStencilState));


	return S_OK;
}


// 우선 깊이 버퍼를 비우고, RTV 를 초기화한다.
// Render 하기전 RTV를 변경하고, Render state 를 변경한다.
void cGBuffer::SwapToGbufferRTV(ID3D11DeviceContext * dc)
{
	float Color[4] = { 1,1,1,0 };
	//	dc->ClearRenderTargetView(m_DepthStencilSRV, Color);
	dc->ClearRenderTargetView(m_DiffuseSpecIntensityRTV, Color);
	dc->ClearRenderTargetView(m_NormalRTV, Color);
	dc->ClearRenderTargetView(m_SpecPowerRTV, Color);

	ID3D11RenderTargetView* rtv[3] = { m_DiffuseSpecIntensityRTV , m_NormalRTV ,m_SpecPowerRTV };
	dc->OMSetRenderTargets(3, rtv, m_DepthStencilDSV);
	dc->ClearDepthStencilView(m_DepthStencilDSV, D3D11_CLEAR_DEPTH| D3D11_CLEAR_STENCIL, 1.0f, 0);
	// 깊이 또한 바꿔준다.
	dc->OMSetDepthStencilState(m_DepthStencilState, 1);
}

void cGBuffer::PostProcessing(ID3D11DeviceContext * dc)
{
	dc->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//static bool bSave = false;
	//if (bSave)
	//{
	//	HRESULT hr;
	//	
	//	HR(D3DX11SaveTextureToFile(dc, m_DiffuseSpecIntensityRTV, , L"c:/testGBuffer0.BMP"));
	//	HR(D3DX11SaveTextureToFile(dc, m_NormalTex, D3DX11_IFF_DDS, L"c:/testGBuffer1.DDS"));
	//	HR(D3DX11SaveTextureToFile(dc, m_SpecPowerRTV, D3DX11_IFF_BMP, L"c:/testGBuffer2.BMP"));
	//	bSave = false;
	//}

	ID3D11RenderTargetView* rt[3] = { NULL, NULL, NULL };
	dc->OMSetRenderTargets(3, rt, m_DepthStencilReadOnlyDSV);

}

void cGBuffer::PrepareForUnpack(ID3D11DeviceContext * dc)
{



}
