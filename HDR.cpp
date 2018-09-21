#include "DXUT.h"
#include "HDR.h"
#include "EffectMGR.h"
#include "Vertex.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "Main.h"

cHDR::cHDR(ID3D11Device* device, ID3D11DeviceContext* dc, UINT width, UINT height) : 
	m_pDevice(device), m_pDC(dc), m_Width(width), m_Height(height), 
	m_pAvgLumUAV(nullptr), m_pAvgLumSRV(nullptr), m_pAvgLumBuffer(nullptr),
	m_pDownScale1DUAV(nullptr), m_pDownScale1DSRV(nullptr),m_pDownScale1DBuffer(nullptr),
	m_fMiddleGray(0.8f), m_fLumWhiteSqr(6.5f)
{
}


cHDR::~cHDR()
{
	Release();

}



void cHDR::Init()
{
	Release();

	m_DownScaleGroups = (UINT)ceil((float)(m_Width * m_Height / (float)16) / 1024.0f);
	// 참고함 : DirectX SDK Example HDR tone Mapping form MS
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate down scaled luminance buffer
	// Down Scale을 진행할 버퍼를 만들고, Down S UAV, SRV를 만들자. ( UAV는 출력용 구조체 버퍼임. )


	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.StructureByteStride = sizeof(float);
	bufferDesc.ByteWidth = m_DownScaleGroups * bufferDesc.StructureByteStride;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	m_pDevice->CreateBuffer(&bufferDesc, NULL, &m_pDownScale1DBuffer);

	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.NumElements = m_DownScaleGroups;
	m_pDevice->CreateUnorderedAccessView(m_pDownScale1DBuffer, &DescUAV, &m_pDownScale1DUAV);

	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
	ZeroMemory(&dsrvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	dsrvd.Format = DXGI_FORMAT_UNKNOWN;
	dsrvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	dsrvd.Buffer.NumElements = m_DownScaleGroups;
	m_pDevice->CreateShaderResourceView(m_pDownScale1DBuffer, &dsrvd, &m_pDownScale1DSRV);

	// 평균 휘도 (빛의 밝기)를 저장할 UAV, SRV를 만들자.

	bufferDesc.ByteWidth = sizeof(float);
	m_pDevice->CreateBuffer(&bufferDesc, NULL, &m_pAvgLumBuffer);

	DescUAV.Buffer.NumElements = 1;
	m_pDevice->CreateUnorderedAccessView(m_pAvgLumBuffer, &DescUAV, &m_pAvgLumUAV);

	dsrvd.Buffer.NumElements = 1;
	m_pDevice->CreateShaderResourceView(m_pAvgLumBuffer, &dsrvd, &m_pAvgLumSRV);
	BuildOffScreenOffSet();
}

void cHDR::Release()
{
	SAFE_RELEASE(m_pDownScale1DBuffer);
	SAFE_RELEASE(m_pDownScale1DUAV);
	SAFE_RELEASE(m_pDownScale1DSRV);
	SAFE_RELEASE(m_pAvgLumBuffer);
	SAFE_RELEASE(m_pAvgLumUAV);
	SAFE_RELEASE(m_pAvgLumSRV);
}

void cHDR::ComputeDownScale(ID3D11ShaderResourceView* pHDRSRV)
{
	// ==================================================
	//                     First Pass
	// ==================================================

	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };

	ID3DX11EffectTechnique* tech = EffectMGR::HDRDownScaleE->GetTech("DownScaleFirst");

	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc(&techDesc);


	UINT downSizeWidth = m_Width /4;
	UINT downSizeHeight = m_Height/4 ;

	EffectMGR::HDRDownScaleE->SetResX(downSizeWidth);
	EffectMGR::HDRDownScaleE->SetResY(downSizeHeight);
	EffectMGR::HDRDownScaleE->SetDomain(downSizeWidth * downSizeHeight);
	EffectMGR::HDRDownScaleE->SetGroupSize(m_DownScaleGroups);


	// Output 
	EffectMGR::HDRDownScaleE->SetDownScaleUAV(m_pDownScale1DUAV);
	// Input
	EffectMGR::HDRDownScaleE->SetHDRInputSRV(pHDRSRV);

	for (int p = 0; p < techDesc.Passes; p++)
	{
		tech->GetPassByIndex(p)->Apply(0, m_pDC);
		m_pDC->Dispatch(m_DownScaleGroups, 1, 1);
	}

	m_pDC->CSSetShaderResources(0, 1, nullSRV);
	m_pDC->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// ==================================================
	//                     Second Pass
	// ==================================================

	tech = EffectMGR::HDRDownScaleE->GetTech("DownScaleSecond");
	tech->GetDesc(&techDesc);


	EffectMGR::HDRDownScaleE->SetResX(downSizeWidth);
	EffectMGR::HDRDownScaleE->SetResY(downSizeHeight);
	EffectMGR::HDRDownScaleE->SetGroupSize(downSizeWidth * downSizeHeight);
	EffectMGR::HDRDownScaleE->SetDomain(m_DownScaleGroups);
	// Output 
	EffectMGR::HDRDownScaleE->SetDownScale1DUAV(m_pAvgLumUAV);
	// Input
	EffectMGR::HDRDownScaleE->SetAvg1DSRV(m_pDownScale1DSRV);

	for (int p = 0; p < techDesc.Passes; p++)
	{
		tech->GetPassByIndex(p)->Apply(0, m_pDC);
		m_pDC->Dispatch(1, 1, 1);
	}

	m_pDC->CSSetShaderResources(0, 1, nullSRV);
	m_pDC->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	m_pDC->CSSetShader(0, 0, 0);

}

void cHDR::ToneMapping(ID3D11ShaderResourceView* pHDRSR)
{
	cCamera* cam = D3DMain::GetInstance()->GetCam();
	m_pDC->IASetInputLayout(InputLayOut::BasicInputLayout);
	m_pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertexs::sBasicVertex);
	UINT offset = 0;

	XMMATRIX identity = XMMatrixIdentity();


	float whiteSQRT = (m_fLumWhiteSqr * m_fMiddleGray);
	whiteSQRT *= whiteSQRT;

	ID3DX11EffectTechnique* tech = EffectMGR::ToneMappingE->GetTech("PostFXPass");

	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc(&techDesc);

	for (int p = 0; p < techDesc.Passes; p++)
	{
		m_pDC->IASetVertexBuffers(0, 1, &m_VB, &stride, &offset);
		m_pDC->IASetIndexBuffer(m_IB, DXGI_FORMAT_R32_UINT, offset);

		EffectMGR::ToneMappingE->SetWorldViewProj(identity);
		EffectMGR::ToneMappingE->SetTexTransform(identity);
		EffectMGR::ToneMappingE->SetInputHDRSRV(pHDRSR);
		EffectMGR::ToneMappingE->SetAvgLumSRV(m_pAvgLumSRV);
		EffectMGR::ToneMappingE->SetMiddelGray(m_fMiddleGray);
		EffectMGR::ToneMappingE->SetWhiteSqrt(whiteSQRT);

		tech->GetPassByIndex(p)->Apply(0, m_pDC);
		m_pDC->DrawIndexed(6, 0,0 );
	}

	m_pDC->VSSetShader(NULL, NULL, 0);
	m_pDC->PSSetShader(NULL, NULL, 0);

}

void cHDR::PostProcessing(ID3D11ShaderResourceView * pHDRSRV, ID3D11RenderTargetView * pLDRRTV)
{
	ID3D11RenderTargetView* rtv[1] = { nullptr };
	m_pDC->OMSetRenderTargets(1, rtv, nullptr);

	ComputeDownScale(pHDRSRV);

	// Do the final pass
	rtv[0] = pLDRRTV;
	m_pDC->OMSetRenderTargets(1, rtv, D3DMain::GetInstance()->GetDSV());
	ToneMapping( pHDRSRV);


}

void cHDR::BuildOffScreenOffSet()
{
		UINT stride = sizeof(Vertexs::sBasicVertex);
		GeometryGenerator geo;
		GeometryGenerator::MeshData shape;

		geo.CreateFullscreenQuad(shape);

		m_OffsetIBCount = shape.Indices.size();
		m_OffsetVBCount = shape.Vertices.size();

		std::vector<Vertexs::sBasicVertex> vertices(m_OffsetVBCount);
		for (int i = 0; i < m_OffsetVBCount; i++)
		{
			vertices[i].Pos = shape.Vertices[i].Position;
			vertices[i].Normal = shape.Vertices[i].Normal;
			vertices[i].Tex = shape.Vertices[i].TexC;
		}

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertexs::sBasicVertex) * m_OffsetVBCount;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = &vertices[0];
		HR(m_pDevice->CreateBuffer(&vbd, &vinitData, &m_VB));

		std::vector<UINT> indices;
		indices.insert(indices.end(), shape.Indices.begin(), shape.Indices.end());

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) *m_OffsetIBCount;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = &indices[0];
		HR(m_pDevice->CreateBuffer(&ibd, &iinitData, &m_IB));

	

}

