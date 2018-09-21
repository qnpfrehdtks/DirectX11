#pragma once
#include "DXUT.h"
#include "Vertex.h"

using namespace DirectX;

class cHeatHaze
{
private:
	ID3D11ShaderResourceView * m_NoiseSRV;

	XMFLOAT3 m_Pos;

	UINT m_Stride;
	UINT m_HeatCount;
	ID3D11Buffer* m_Buffer;
	std::vector<Vertexs::sSimple> m_Pts;
	float m_Time;

public:
	cHeatHaze();
	~cHeatHaze();

	void Update(float dt);

	void Init(ID3D11Device* device, ID3D11DeviceContext* pDC, XMFLOAT3 Pos);
	void BuildBuffer(ID3D11Device* device, ID3D11DeviceContext* pDC, XMFLOAT3 Pos);
	void DrawHeatHaze( ID3D11Device * device, ID3D11DeviceContext * pDC, ID3D11ShaderResourceView* srv, CXMMATRIX vp);

	
};

