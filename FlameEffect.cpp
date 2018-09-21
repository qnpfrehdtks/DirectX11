#include "DXUT.h"
#include "FlameEffect.h"
#include "TextureResourceMGR.h"
#include "Vertex.h"
#include "EffectMGR.h"
#include "Camera.h"
#include "LightMGR.h"
#include "RenderState.h"

cFlameEffect::cFlameEffect() : m_CurTime(0.0f), m_DurationTime(0.75), m_IntervalTime(0.0f), m_CurIdx(0)
{
	
}


cFlameEffect::~cFlameEffect()
{
}

void cFlameEffect::BuildBuffer(ID3D11Device * pDevice, ID3D11DeviceContext * pDC)
{
	/*UINT ByteWidth;
	D3D11_USAGE Usage;
	UINT BindFlags;
	UINT CPUAccessFlags;
	UINT MiscFlags;
	UINT StructureByteStride;*/

	//m_Pts.Pos = XMFLOAT3(97.8f, 32.2f, 44.78f);
	//m_Pts.Color = XMFLOAT4(99, 99, 99,1);

	D3D11_BUFFER_DESC bd;
	bd.ByteWidth = sizeof(Vertexs::sSimple) * m_EffectCount;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA sub;
	sub.pSysMem = &((m_Pts));

	HR(pDevice->CreateBuffer(&bd, &sub, &m_Buffer));
}

void cFlameEffect::Init(ID3D11Device * pDevice, ID3D11DeviceContext * pDC, UINT idx, XMFLOAT3* pos, UINT flameCount)
{

	// ∫“¿« PointLight
	

	m_EffectCount = flameCount;

	for (UINT i = 0; i < m_EffectCount; i++) {
		m_Pts[i].Pos = XMFLOAT3(pos[i].x, pos[i].y, pos[i].z);
		m_Pts[i].Color = XMFLOAT4(0, 0, 0, 1);

		LightMGR::GetInstance()->CreatePointLight(
			m_Pts[i].Pos,
			XMFLOAT4(255, 70, 0, 1),
			XMFLOAT4(255, 70, 0, 1),
			XMFLOAT4(255, 40, 40, 1),
			80, XMFLOAT3(0.15f, 0.15f, 0.15f));

	}

	m_MaxIdx = idx;
	m_IntervalTime = m_DurationTime / (float)m_MaxIdx;

	m_NoiseSRV = TextureResourceMGR::GetInstance()->CreateSRVFromWIC(pDevice, L"Textures/NoiseMap.png");

	m_Stride = sizeof(Vertexs::sSimple);
	std::vector<std::wstring> str(idx);

	for (int i = 0; i < idx; i++)
	{
		std::string front = "Textures/FireAnim/fire1_ ";

		char* num = new char[5];
		itoa(i + 1, num, 10);
		std::string NumStr = num;

		for (int i = 0; i < 2 - NumStr.size(); i++)
		{
			front = front + "0";
		}

		front = front + NumStr;

		std::string back = ".png";
		std::string result = front + back;

		std::wstring result2 = std::wstring(result.begin(), result.end());

		str[i] = result2;

		delete[] num;
	}

	m_SpriteSRV = TextureResourceMGR::GetInstance()->CreateSRVArrFromDDS(pDevice, pDC, str);

	BuildBuffer(pDevice, pDC);


}

void cFlameEffect::Draw(ID3D11Device * pDevice, ID3D11DeviceContext * pDC, cCamera * cam, CXMMATRIX VP)
{
	UINT offset = 0;

	pDC->IASetInputLayout(InputLayOut::SimpleInputLayout);
	pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	pDC->IASetVertexBuffers(0, 1, &m_Buffer, &m_Stride, &offset);

	ID3DX11EffectTechnique* tech = EffectMGR::SpriteE->GetTech("SpriteBasic");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);


	XMMATRIX P = cam->GetProjMat();
	XMMATRIX V = cam->GetViewMat();

	EffectMGR::SpriteE->SetNoiseMap(m_NoiseSRV);
	EffectMGR::SpriteE->SetTexArray(m_SpriteSRV);
	EffectMGR::SpriteE->SetIndex(m_CurIdx);
	EffectMGR::SpriteE->SetViewProj(V * P);

	//EffectMGR::gBufferE->SetViewProj(viewProj);
	EffectMGR::SpriteE->SetEyePos(cam->GetPos());

	float belndFactor[4] = { 0,0,0,0 };

	float ref[4] = { 0,0,0,0 };

		

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		pDC->OMSetDepthStencilState(RenderStates::ReflectionDSS, 1);
		pDC->OMSetBlendState(RenderStates::TransparentBS, belndFactor,0xffffffff);

		tech->GetPassByIndex(p)->Apply(0, pDC);
		pDC->Draw(m_EffectCount, 0);
	}

	pDC->OMSetBlendState(0, belndFactor, 0xffffffff);
	pDC->OMSetDepthStencilState(0, 0);

}

void cFlameEffect::Update(float dt)
{
	m_CurTime += dt;

	if (m_CurTime >= m_IntervalTime)
	{
		m_CurTime = 0.0f;

		if (m_CurIdx >= (m_MaxIdx - 1))
		{
			m_CurIdx = 0;
		}
		else
		{
			m_CurIdx++;
		}
	}

}

