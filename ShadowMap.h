#pragma once
#include "Main.h"
#include "SkinModel.h"
using namespace DirectX;


class cSimpleShape;
class cBasicShape;


class cShadowMap
{
public:
	cShadowMap( ID3D11Device* device, ID3D11DeviceContext* dc, float width, float height);
	~cShadowMap();

	// =====================================
	//                  GET
	// =====================================
public: 

	void Init();
	void UpdateShadow(float dt);

	ID3D11ShaderResourceView * GetShadowMap() { return m_SRV; }
	XMFLOAT4X4 GetShadowTransform()           { return m_ShadowTransform;  }

private:
	ID3D11Device * m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;

	XMFLOAT4X4 m_ShadowTransform;
	XMFLOAT4X4 m_LightView;
	XMFLOAT4X4 m_LightProj;

	ID3D11ShaderResourceView * m_SRV;
	ID3D11RenderTargetView *   m_RTV;
	ID3D11DepthStencilView*    m_DSV;
	D3D11_VIEWPORT m_ViewPort;

	float m_height;
	float m_width;

	XMFLOAT3 m_center;
	float    m_radius;

	std::vector<cBasicShape*> m_Shapes;

private:
	void CreateShadowTexture(ID3D11Device* device, ID3D11DeviceContext* dc);
	void BuildShadowTransform();

public:
	void SwapRenderTargetToShadow();
	void DrawToShadowMap(std::vector<cBasicShape*>& shapes, cCamera* cam,  UINT count = 1);
	void DrawToShadowMap(sSkinModellInstance& skinModel, cCamera* cam, UINT count = 1 , BOOL hasAni = false);
	void InsertShape(cBasicShape * shape);


};

