#include "DXUT.h"
#include "Glow.h"
#include "Vertex.h"
#include "LightMGR.h"
#include "EffectMGR.h"
#include "DDSTextureLoader.h"

cGlow::cGlow() : m_Stride(0), m_GlowCount(0)
{
	
}


cGlow::~cGlow()
{
}

void cGlow::Init(ID3D11Device* device, ID3D11DeviceContext * pDC)
{
//	EffectMGR::GlowE->SetSize(XMFLOAT4(1, 1, 1, 1));
	m_Stride = sizeof(Vertexs::sSimple);
	BuildGlowBuffer(device, pDC);
	HR(CreateDDSTextureFromFile(device, L"Textures/GlowCircle.dds", nullptr, &g_GlowMaskSRV));
	
}

void cGlow::BuildGlowBuffer(ID3D11Device* device, ID3D11DeviceContext * pDC)
{
	/*UINT ByteWidth;
	D3D11_USAGE Usage;
	UINT BindFlags;
	UINT CPUAccessFlags;
	UINT MiscFlags;
	UINT StructureByteStride;*/

	m_GlowCount = LightMGR::GetInstance()->GetPointLightCount();
	m_Pts = LightMGR::GetInstance()->GetPtLightsPoss();

	D3D11_BUFFER_DESC bd;
	bd.ByteWidth = sizeof(Vertexs::sSimple) * m_GlowCount;
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA sub;
	sub.pSysMem = &((*m_Pts)[0]);

	HR(device->CreateBuffer(&bd, &sub, &m_Buffer));




}

void cGlow::DrawGlowMap(ID3D11DeviceContext * pDC, XMFLOAT3 EyePosW, CXMMATRIX vp)
{

	UINT offset = 0;

	pDC->IASetInputLayout(InputLayOut::SimpleInputLayout);
	pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	pDC->IASetVertexBuffers(0, 1, &m_Buffer, &m_Stride, &offset);
	
	ID3DX11EffectTechnique* tech = EffectMGR::gBufferE->GetTech("GBufferGlow");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);


	//EffectMGR::gBufferE->SetS("GBufferGlow");
	EffectMGR::gBufferE->SetIsBumped(false);
	EffectMGR::gBufferE->SetIsSpeced(false);
	EffectMGR::gBufferE->SetIsMask(true);
	EffectMGR::gBufferE->SetIsSSR(false);
	EffectMGR::gBufferE->SetMaskMap(g_GlowMaskSRV);
	EffectMGR::gBufferE->SetDiffuseMap(nullptr);
	EffectMGR::gBufferE->SetSpecMap(nullptr);
	EffectMGR::gBufferE->SetNormalMap(nullptr);

	EffectMGR::gBufferE->SetViewProj(vp);
	EffectMGR::gBufferE->SetEyePosW(EyePosW);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, pDC);
		pDC->Draw(m_GlowCount, 0);
	}


}

void cGlow::Update(ID3D11DeviceContext * dc)
{
	D3D11_MAPPED_SUBRESOURCE resource;
	dc->Map(m_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, &((*m_Pts)[0]), sizeof(Vertexs::sSimple) * m_GlowCount);
	dc->Unmap(m_Buffer, 0);
}
