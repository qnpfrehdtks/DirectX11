#pragma once
#include "DXUT.h"
#include "Main.h"
#include "SkinModel.h"
#include "Camera.h"

class cPointLightShadowMap
{

private:
	cCamera m_Cam[6];


	std::vector<ID3D11DepthStencilView*> m_ArraySlices;

	D3D11_VIEWPORT m_ViewPort[6];

	ID3D11DepthStencilView* m_DSV;
	ID3D11RenderTargetView* m_RTV[6];
	ID3D11ShaderResourceView * m_SRV;
	ID3D11Texture2D* m_Tex[6];

	UINT m_Width;
	UINT m_Height;


public:
	cPointLightShadowMap();
	~cPointLightShadowMap();


	ID3D11ShaderResourceView* GetCubeMap() { return m_SRV; }

	void Init(ID3D11Device* device, ID3D11DeviceContext* dc, UINT width, UINT height);
	void CreateTextureView(ID3D11Device* device, ID3D11DeviceContext* dc);

	void BuildCubeFaceCam(float x, float y, float z);
	void DarwDepthCubeMap(ID3D11Device* device, ID3D11DeviceContext* dc, sSkinModellInstance & skinModel);

	void DrawToCubeMap(sSkinModellInstance& skinModel, ID3D11DeviceContext * dc, cCamera* cam, UINT count = 1, BOOL hasAni = false);


};

