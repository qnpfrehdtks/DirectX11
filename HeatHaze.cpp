#include "DXUT.h"
#include "HeatHaze.h"
#include "EffectMGR.h"
#include "Vertex.h"
#include "Camera.h"
#include "Main.h"
#include "TextureResourceMGR.h"
#include "RenderState.h"

cHeatHaze::cHeatHaze() : m_Time(0)
{
}


cHeatHaze::~cHeatHaze()
{
}

void cHeatHaze::Update(float dt)
{

	m_Time += dt;
	if (m_Time > 2.0f)
	{
		m_Time = 0;
	}

}

void cHeatHaze::Init(ID3D11Device * device, ID3D11DeviceContext * pDC, XMFLOAT3 Pos)
{
	m_Stride = sizeof(Vertexs::sSimple);
	

	m_NoiseSRV = TextureResourceMGR::GetInstance()->CreateSRVFromWIC(device, L"Textures/NoiseMap.png");


	BuildBuffer(device, pDC, Pos);
}

void cHeatHaze::BuildBuffer(ID3D11Device * device, ID3D11DeviceContext * pDC, XMFLOAT3 Pos)
{
	/*UINT ByteWidth;
	D3D11_USAGE Usage;
	UINT BindFlags;
	UINT CPUAccessFlags;
	UINT MiscFlags;
	UINT StructureByteStride;*/
	
	m_HeatCount = 1;
	Vertexs::sSimple pos;
	pos.Pos = Pos;
	pos.Color = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_Pts.push_back(pos);

	D3D11_BUFFER_DESC bd;
	bd.ByteWidth = sizeof(Vertexs::sSimple) * m_HeatCount;
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA sub;
	sub.pSysMem = &((m_Pts)[0]);

	HR(device->CreateBuffer(&bd, &sub, &m_Buffer));

}

void cHeatHaze::DrawHeatHaze(ID3D11Device * device, ID3D11DeviceContext * pDC, ID3D11ShaderResourceView* srv, CXMMATRIX vp)
{
	UINT offset = 0;

	pDC->IASetInputLayout(InputLayOut::SimpleInputLayout);
	pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	pDC->IASetVertexBuffers(0, 1, &m_Buffer, &m_Stride, &offset);

	ID3DX11EffectTechnique* tech = EffectMGR::HazeE->GetTech("HazeBasic");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);

	//EffectMGR::gBufferE->SetS("GBufferGlow");
	cCamera* cam = D3DMain::GetInstance()->GetCam();


	EffectMGR::HazeE->SetNoiseMap(m_NoiseSRV);
	EffectMGR::HazeE->SetDiffuseMap(srv);
	EffectMGR::HazeE->SetEyePos(cam->GetPos());
	EffectMGR::HazeE->SetViewProj(vp);
	EffectMGR::HazeE->SetTime(m_Time);

	float ref[4] = { 0,0,0,0 };

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		pDC->OMSetDepthStencilState(RenderStates::MirrorDSS, 1);

		tech->GetPassByIndex(p)->Apply(0, pDC);
		pDC->Draw(m_HeatCount, 0);
	}

	pDC->OMSetDepthStencilState(0, 0);


}
