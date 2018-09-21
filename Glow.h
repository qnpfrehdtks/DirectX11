#pragma once
#include "DXUT.h"
#include "Vertex.h"

using namespace DirectX;


class cGlow
{
public:
	cGlow();
	~cGlow();

	void Init(ID3D11Device* device, ID3D11DeviceContext * pDC);
	void BuildGlowBuffer(ID3D11Device* device,  ID3D11DeviceContext* pDC);
	void DrawGlowMap(ID3D11DeviceContext* pDC, XMFLOAT3 EyePosW, CXMMATRIX vp);

	void Update(ID3D11DeviceContext* dc);

private:
	ID3D11Buffer * m_Buffer;

	UINT m_Stride;
	UINT m_GlowCount;

	std::vector<Vertexs::sSimple>* m_Pts;
	ID3D11ShaderResourceView* g_GlowMaskSRV;

	

};

