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
	// 순서 없는 접근 뷰가 무엇인지 이야기하는 외국글.
	// 출력 자원을 계산 쉐이더에 묶기 위해서는 순서 없는 접근 뷰가 필요 하다.
	// ID3D11UnorederedAccessView 인터페이스로 접근한다.
	SAFE_RELEASE(m_BlurOutputTexSRV);
	SAFE_RELEASE(m_BlurOutputTexUAV);

	// UAV는 압축 텍스처 사용 불가.
	D3D11_TEXTURE2D_DESC tbd;
	tbd.Width = m_Width;
	tbd.Height = m_Height;
	tbd.MipLevels = 1;
	tbd.ArraySize = 1;
	tbd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	tbd.SampleDesc.Count = 1;
	tbd.SampleDesc.Quality = 0;
	tbd.Usage = D3D11_USAGE_DEFAULT;
	

	// 순서 없는 뷰를 통해서 계산 쉐이던 파이프라인에 묶을 텍스처는 반드시 플래그를 설정 해야 한다.
	// 하나는 SRV를 위한, 그리고 또 하나는 Unordered_Access를 위한 플래그.
	tbd.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	tbd.CPUAccessFlags = 0;
	tbd.MiscFlags = 0;


	// 텍스처 2D 생성.
	ID3D11Texture2D* blurTex = 0;
	HR(m_pDevice->CreateTexture2D(&tbd, 0, &blurTex));

	// 쉐이더 자원 뷰로 생성하기 위해 구조체 서술.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	srvd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0;
	srvd.Texture2D.MipLevels = 1;

	// Shader 자원뷰 생성.
	HR(m_pDevice->CreateShaderResourceView(blurTex, &srvd, &m_BlurOutputTexSRV));


	// 이번에는 계산 쉐이더의 출력을 위해, UAV로 묶어준다.
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
	uavd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavd.Texture2D.MipSlice = 0;
	// View 생성 후에는 Effect에 묶어 주어야 하는데...
	HR(m_pDevice->CreateUnorderedAccessView(blurTex, &uavd, &m_BlurOutputTexUAV));

	BuildOffScreenViews();

	SAFE_RELEASE(blurTex);

}

void cBlurFilter::ComputeBlur(UINT blurCount, cNormalDepthMap* dmap)
{

	ID3DX11EffectTechnique* horz = EffectMGR::BlurE->GetTech("HorzBlur");
	ID3DX11EffectTechnique* vert = EffectMGR::BlurE->GetTech("VertBlur");

	// 화면 밖 텍스처에 흐리기를 하기 위해 CS를 실행한다.
	// Blur 수 만큼 실행.
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

		// 계산 쉐이더에서 입력 텍스처를 계산 쉐이더에서 우선 띠어내고.
		ID3D11ShaderResourceView* srv[1] = { 0 };
		m_pDC->CSSetShaderResources(0, 1, srv);

		// 계산 쉐이더에서 출력 을 다음 패스의 입력으로 사용한다.
		// 하나의 자원을 동시에 읽고 쓸수 없다.
		ID3D11UnorderedAccessView* uav[1] = { 0 };
		m_pDC->CSSetUnorderedAccessViews(0, 1, uav, 0);

		// 수직 흐리기 Pass 로 넘어가자.
		vert->GetDesc(&techDesc);
		for (int j = 0; j< techDesc.Passes; j++)
		{
			EffectMGR::BlurE->SetInputSRV(m_BlurOutputTexSRV);
			EffectMGR::BlurE->SetDepthSRV(dmap->GetDepthSRV());
			EffectMGR::BlurE->SetOutUAV(m_OffScreenUAV);

			// 최종적으로  InputUAV에 모든 흐려진 결과가 나옴.

			vert->GetPassByIndex(j)->Apply(0, m_pDC);

			UINT TreadCountY = (UINT)ceilf(m_Height / 256.0f);

			// 
			m_pDC->Dispatch(m_Width, TreadCountY, 1);
		}

		m_pDC->CSSetShaderResources(0, 1, srv);
		m_pDC->CSSetUnorderedAccessViews(0, 1, uav, 0);
	}
	// 계산 쉐이더를 비호라성화 한다.
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

	// 텍스처 재생성각띠~
	D3D11_TEXTURE2D_DESC texDesc;

	texDesc.Width = m_Width ;
	texDesc.Height = m_Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;

	// 텍스처의 용도를 뜻하는 필드, 
	// 자원을 읽고 써야 한다면 이 용도를 설정한다.
	// CPU는 쓸 수 없다.
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	// 렌더 타겟으로도, Shader 자원으로도 쓸 수 있다.
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* offscreenTex = 0;
	HR(m_pDevice->CreateTexture2D(&texDesc, 0, &offscreenTex));


	// Texture를 쉐이더 자원 뷰, 입력으로도 넣고,
	HR(m_pDevice->CreateShaderResourceView(offscreenTex, 0, &m_OffScreenSRV));
	HR(m_pDevice->CreateRenderTargetView(offscreenTex, 0, &m_OffScreenRTV));
	// 계산 쉐이더에 기록할 자원을 묶을 UAV.
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



