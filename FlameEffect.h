#pragma once
#include "DXUT.h"
#include "Camera.h"
#include "Vertex.h"
class cFlameEffect
{

private:
	ID3D11ShaderResourceView * m_SpriteSRV;

	float m_CurTime;
	float m_DurationTime;
	float m_IntervalTime;

	UINT m_CurIdx;
	UINT m_MaxIdx;

	UINT m_EffectCount;
	UINT m_Stride;
	Vertexs::sSimple m_Pts[4];

	ID3D11Buffer* m_Buffer;
	ID3D11ShaderResourceView * m_NoiseSRV;

public:
	cFlameEffect();
	~cFlameEffect();

	void BuildBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDC);
	void Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDC, UINT idx, XMFLOAT3* pos, UINT flameCount);
	void Draw(ID3D11Device* pDevice, ID3D11DeviceContext* pDC, cCamera* cam, CXMMATRIX VP);
	void Update(float dt);

};

