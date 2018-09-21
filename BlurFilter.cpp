#include "DXUT.h"
#include "BlurFilter.h"
#include "GeometryGenerator.h"
#include "Vertex.h"




cBlurFilter::cBlurFilter( ID3D11Device* device, ID3D11DeviceContext* dc,  UINT width, UINT height) :
	m_pDevice(device), m_pDC(dc),
	m_BlurOutputTexSRV(nullptr), m_BlurOutputTexUAV(nullptr),
	m_OffScreenSRV(nullptr),m_OffScreenUAV(nullptr), m_OffScreenRTV(nullptr),
	m_OffScreenVB(nullptr), m_OffScreenIB(nullptr),
	m_Width(width), m_Height(height)
	, m_OffsetIBCount(0), m_OffsetVBCount(0)
{
}


cBlurFilter::~cBlurFilter()
{
}

void cBlurFilter::Init()
{
	// https://www.gamedev.net/forums/topic/669746-uses-for-unordered-access-views/
	// ���� ���� ���� �䰡 �������� �̾߱��ϴ� �ܱ���.
	// ��� �ڿ��� ��� ���̴��� ���� ���ؼ��� ���� ���� ���� �䰡 �ʿ� �ϴ�.
	// ID3D11UnorederedAccessView �������̽��� �����Ѵ�.
	SAFE_RELEASE(m_BlurOutputTexSRV);
	SAFE_RELEASE(m_BlurOutputTexUAV);

	// UAV�� ���� �ؽ�ó ��� �Ұ�.
	D3D11_TEXTURE2D_DESC tbd;
	tbd.Width = m_Width;
	tbd.Height = m_Height;
	tbd.MipLevels = 1;
	tbd.ArraySize = 1;
	tbd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	tbd.SampleDesc.Count = 1;
	tbd.SampleDesc.Quality = 0;
	tbd.Usage = D3D11_USAGE_DEFAULT;
	

	// ���� ���� �並 ���ؼ� ��� ���̴� ���������ο� ���� �ؽ�ó�� �ݵ�� �÷��׸� ���� �ؾ� �Ѵ�.
	// �ϳ��� SRV�� ����, �׸��� �� �ϳ��� Unordered_Access�� ���� �÷���.
	tbd.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	tbd.CPUAccessFlags = 0;
	tbd.MiscFlags = 0;


	// �ؽ�ó 2D ����.
	ID3D11Texture2D* blurTex = 0;
	HR(m_pDevice->CreateTexture2D(&tbd, 0, &blurTex));

	// ���̴� �ڿ� ��� �����ϱ� ���� ����ü ����.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	srvd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0;
	srvd.Texture2D.MipLevels = 1;

	// Shader �ڿ��� ����.
	HR(m_pDevice->CreateShaderResourceView(blurTex, &srvd, &m_BlurOutputTexSRV));


	// �̹����� ��� ���̴��� ����� ����, UAV�� �����ش�.
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
	uavd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavd.Texture2D.MipSlice = 0;
	// View ���� �Ŀ��� Effect�� ���� �־�� �ϴµ�...
	HR(m_pDevice->CreateUnorderedAccessView(blurTex, &uavd, &m_BlurOutputTexUAV));

	BuildOffScreenViews();

	SAFE_RELEASE(blurTex);

}

void cBlurFilter::ComputeBlur(UINT blurCount, cNormalDepthMap* dmap)
{

	ID3DX11EffectTechnique* horz = EffectMGR::BlurE->GetTech("HorzBlur");
	ID3DX11EffectTechnique* vert = EffectMGR::BlurE->GetTech("VertBlur");

	// ȭ�� �� �ؽ�ó�� �帮�⸦ �ϱ� ���� CS�� �����Ѵ�.
	// Blur �� ��ŭ ����.
	for (int i = 0; i < blurCount; i++)
	{
		D3DX11_TECHNIQUE_DESC techDesc;
		horz->GetDesc(&techDesc);
		for (int j = 0; j < techDesc.Passes; j++)
		{
			EffectMGR::BlurE->SetInputSRV(m_OffScreenSRV);
			EffectMGR::BlurE->SetDepthSRV(dmap->GetDepthSRV());
			EffectMGR::BlurE->SetOutUAV(m_BlurOutputTexUAV);

			horz->GetPassByIndex(j)->Apply(0, m_pDC);

			UINT TreadCountX = (UINT)ceilf(m_Width / 256.0f);
			// 
			m_pDC->Dispatch(TreadCountX, m_Height, 1);
		}

		// ��� ���̴����� �Է� �ؽ�ó�� ��� ���̴����� �켱 ����.
		ID3D11ShaderResourceView* srv[1] = { 0 };
		m_pDC->CSSetShaderResources(0, 1, srv);

		// ��� ���̴����� ��� �� ���� �н��� �Է����� ����Ѵ�.
		// �ϳ��� �ڿ��� ���ÿ� �а� ���� ����.
		ID3D11UnorderedAccessView* uav[1] = { 0 };
		m_pDC->CSSetUnorderedAccessViews(0, 1, uav, 0);

		// ���� �帮�� Pass �� �Ѿ��.
		vert->GetDesc(&techDesc);
		for (int j = 0; j< techDesc.Passes; j++)
		{
			EffectMGR::BlurE->SetInputSRV(m_BlurOutputTexSRV);
			EffectMGR::BlurE->SetDepthSRV(dmap->GetDepthSRV());
			EffectMGR::BlurE->SetOutUAV(m_OffScreenUAV);

			// ����������  InputUAV�� ��� ����� ����� ����.

			vert->GetPassByIndex(j)->Apply(0, m_pDC);

			UINT TreadCountY = (UINT)ceilf(m_Height / 256.0f);

			// 
			m_pDC->Dispatch(m_Width, TreadCountY, 1);
		}

		m_pDC->CSSetShaderResources(0, 1, srv);
		m_pDC->CSSetUnorderedAccessViews(0, 1, uav, 0);
	}
	// ��� ���̴��� ��ȣ��ȭ �Ѵ�.
	m_pDC->CSSetShader(0, 0, 0);

}

void cBlurFilter::BuildOffScreenGeometry()
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
	HR(m_pDevice->CreateBuffer(&vbd, &vinitData, &m_OffScreenVB));

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
	HR(m_pDevice->CreateBuffer(&ibd, &iinitData, &m_OffScreenIB));

}

void cBlurFilter::BuildOffScreenViews()
{
	SAFE_RELEASE(m_OffScreenRTV);
	SAFE_RELEASE(m_OffScreenSRV);
	SAFE_RELEASE(m_OffScreenUAV);

	// �ؽ�ó ���������~
	D3D11_TEXTURE2D_DESC texDesc;

	texDesc.Width = m_Width ;
	texDesc.Height = m_Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;

	// �ؽ�ó�� �뵵�� ���ϴ� �ʵ�, 
	// �ڿ��� �а� ��� �Ѵٸ� �� �뵵�� �����Ѵ�.
	// CPU�� �� �� ����.
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	// ���� Ÿ�����ε�, Shader �ڿ����ε� �� �� �ִ�.
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* offscreenTex = 0;
	HR(m_pDevice->CreateTexture2D(&texDesc, 0, &offscreenTex));


	// Texture�� ���̴� �ڿ� ��, �Է����ε� �ְ�,
	HR(m_pDevice->CreateShaderResourceView(offscreenTex, 0, &m_OffScreenSRV));
	HR(m_pDevice->CreateRenderTargetView(offscreenTex, 0, &m_OffScreenRTV));
	// ��� ���̴��� ����� �ڿ��� ���� UAV.
	HR(m_pDevice->CreateUnorderedAccessView(offscreenTex, 0, &m_OffScreenUAV));

	// View saves a reference to the texture so we can release our reference.
	SAFE_RELEASE(offscreenTex);
}

void cBlurFilter::SwapRTV(ID3D11DepthStencilView* dsv)
{
	ID3D11RenderTargetView* rtv[1] = { m_OffScreenRTV };
	m_pDC->OMSetRenderTargets(1, rtv, dsv);
	m_pDC->ClearRenderTargetView(m_OffScreenRTV, Colors::Aqua);
	m_pDC->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void cBlurFilter::DrawScreen()
{
	m_pDC->IASetInputLayout(InputLayOut::BasicInputLayout);
	m_pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertexs::sBasicVertex);
	UINT offset = 0;

	XMMATRIX identity = XMMatrixIdentity();

	ID3DX11EffectTechnique* tech = EffectMGR::BasicE->GetTech("TexDirLight");

	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc(&techDesc);
	for (int p = 0; p < techDesc.Passes; p++)
	{
		m_pDC->IASetVertexBuffers(0, 1, &m_OffScreenVB, &stride, &offset);
		m_pDC->IASetIndexBuffer(m_OffScreenIB, DXGI_FORMAT_R32_UINT, offset);

		EffectMGR::BasicE->SetWorld(identity ); 
		EffectMGR::BasicE->SetWorldInvTranspose(identity);
		EffectMGR::BasicE->SetWorldViewProj(identity);
		EffectMGR::BasicE->SetTexTransform(identity);
		EffectMGR::BasicE->SetDiffuseMap(m_OffScreenSRV);

		tech->GetPassByIndex(p)->Apply(0, m_pDC);
		m_pDC->DrawIndexed(6, 0, 0);
	}

}



