#include "DXUT.h"
#include "BloomFilter.h"
#include "EffectMGR.h"
#include "Vertex.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "Main.h"

cBloomFilter::cBloomFilter(ID3D11Device* device, ID3D11DeviceContext* dc, UINT width, UINT height) :
	m_pDevice(device), m_pDC(dc), m_Width(width), m_Height(height),
	m_pAvgLumUAV(nullptr), m_pAvgLumSRV(nullptr), m_pAvgLumBuffer(nullptr),
	m_pDownScale1DUAV(nullptr), m_pDownScale1DSRV(nullptr), m_pDownScale1DBuffer(nullptr),
	m_fMiddleGray(0.8f), m_fLumWhiteSqr(6.5f), m_fBloomThreshold(2.0f), m_fBloomScale(1.0f)
{
}


cBloomFilter::~cBloomFilter()
{
	Release();

}



void cBloomFilter::Init()
{
	Release();

	m_DownScaleGroups = (UINT)ceil((float)(m_Width * m_Height / (float)16) / 1024.0f);

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Down Scaled 된 Render Tartget 저장.
	D3D11_TEXTURE2D_DESC dtd = {
		m_Width / 4, //UINT Width;
		m_Height / 4, //UINT Height;
		1, //UINT MipLevels;
		1, //UINT ArraySize;
		DXGI_FORMAT_R16G16B16A16_FLOAT, //DXGI_FORMAT Format;
		1, //DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	};
	ID3D11Texture2D* blurTex = 0;
	HR(m_pDevice->CreateTexture2D(&dtd, NULL, &blurTex));

	// Create the resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
//	ZeroMemory(&dsrvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	dsrvd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	dsrvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	dsrvd.Texture2D.MostDetailedMip = 0;
	dsrvd.Texture2D.MipLevels = 1;
	HR(m_pDevice->CreateShaderResourceView(blurTex, &dsrvd, &m_pDownScaleSRV));
	
	// Create the UAVs
	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
//	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.NumElements = m_Width * m_Height / 16;
	HR(m_pDevice->CreateUnorderedAccessView(blurTex, &DescUAV, &m_pDownScaleUAV));

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate temporary target
	HR(m_pDevice->CreateTexture2D(&dtd, NULL, &m_pTempTex[0]));
	HR(m_pDevice->CreateShaderResourceView(m_pTempTex[0], &dsrvd, &m_pTempSRV[0]));
	HR(m_pDevice->CreateUnorderedAccessView(m_pTempTex[0], &DescUAV, &m_pTempUAV[0]));
	HR(m_pDevice->CreateRenderTargetView(m_pTempTex[0], 0, &m_pTempRT[0]));

	HR(m_pDevice->CreateTexture2D(&dtd, NULL, &m_pTempTex[1]));
	HR(m_pDevice->CreateShaderResourceView(m_pTempTex[1], &dsrvd, &m_pTempSRV[1]));
	HR(m_pDevice->CreateUnorderedAccessView(m_pTempTex[1], &DescUAV, &m_pTempUAV[1]));
	HR(m_pDevice->CreateRenderTargetView(m_pTempTex[1], 0, &m_pTempRT[1]));
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate bloom target

	HR(m_pDevice->CreateTexture2D(&dtd, NULL, &m_pBloomRT));
	HR(m_pDevice->CreateShaderResourceView(m_pBloomRT, &dsrvd, &m_pBloomSRV));
	HR(m_pDevice->CreateUnorderedAccessView(m_pBloomRT, &DescUAV, &m_pBloomUAV));

	// 참고함 : DirectX SDK Example HDR tone Mapping form MS
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate down scaled luminance buffer
	// Down Scale을 진행할 버퍼를 만들고, Down S UAV, SRV를 만들자. ( UAV는 출력용 구조체 버퍼임. )


	D3D11_BUFFER_DESC bufferDesc;
//	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.StructureByteStride = sizeof(float);
	bufferDesc.ByteWidth = m_DownScaleGroups * bufferDesc.StructureByteStride;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_pDevice->CreateBuffer(&bufferDesc, NULL, &m_pDownScale1DBuffer);

	//ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.NumElements = m_DownScaleGroups;
	m_pDevice->CreateUnorderedAccessView(m_pDownScale1DBuffer, &DescUAV, &m_pDownScale1DUAV);

	//ZeroMemory(&dsrvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	dsrvd.Format = DXGI_FORMAT_UNKNOWN;
	dsrvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	dsrvd.Buffer.FirstElement = 0;
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
	BuildResultTexture();
}

void cBloomFilter::Release()
{
	SAFE_RELEASE(m_pDownScale1DBuffer);
	SAFE_RELEASE(m_pDownScale1DUAV);
	SAFE_RELEASE(m_pDownScale1DSRV);
	SAFE_RELEASE(m_pAvgLumBuffer);
	SAFE_RELEASE(m_pAvgLumUAV);
	SAFE_RELEASE(m_pAvgLumSRV);
}
void cBloomFilter::PostProcessing(ID3D11ShaderResourceView * pHDRSRV, ID3D11RenderTargetView * pLDRRTV, ID3D11DepthStencilView* pDSV)
{
	ID3D11RenderTargetView* rtv[2] = { nullptr,nullptr };
	m_pDC->OMSetRenderTargets(1, rtv, nullptr);

	ComputeDownScale(pHDRSRV);

	// Bloom
	Bloom();
	// Blur the bloom values
	Blur(m_pTempSRV[0], m_pBloomUAV, 4);

	rtv[0] = pLDRRTV;
	rtv[1] = m_pResultRTV;
	m_pDC->OMSetRenderTargets(2, rtv, pDSV);

	ToneMapping(pHDRSRV);
}


void cBloomFilter::ComputeDownScale(ID3D11ShaderResourceView* pHDRSRV)
{
	// ==================================================
	//                     First Pass
	// ==================================================

	ID3D11UnorderedAccessView* nullUAV[2] = { nullptr };
	ID3D11ShaderResourceView* nullSRV[2] = { nullptr };

	ID3DX11EffectTechnique* tech = EffectMGR::HDRDownScaleE->GetTech("DownScaleFirst");

	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc(&techDesc);


	UINT downSizeWidth = m_Width / 4;
	UINT downSizeHeight = m_Height / 4;

	EffectMGR::HDRDownScaleE->SetResX(downSizeWidth);
	EffectMGR::HDRDownScaleE->SetResY(downSizeHeight);
	EffectMGR::HDRDownScaleE->SetDomain(downSizeWidth * downSizeHeight);
	EffectMGR::HDRDownScaleE->SetGroupSize(m_DownScaleGroups);

	// Output 
	EffectMGR::HDRDownScaleE->SetDownScale1DUAV(m_pDownScale1DUAV);
	EffectMGR::HDRDownScaleE->SetDownScaleUAV(m_pDownScaleUAV);
	// Input
	EffectMGR::HDRDownScaleE->SetHDRInputSRV(pHDRSRV);

	for (int p = 0; p < techDesc.Passes; p++)
	{
		tech->GetPassByIndex(p)->Apply(0, m_pDC);
		m_pDC->Dispatch(m_DownScaleGroups, 1, 1);
	}

	m_pDC->CSSetShaderResources(0, 2, nullSRV);
	m_pDC->CSSetUnorderedAccessViews(0, 2, nullUAV, 0);

	// ==================================================
	//                     Second Pass
	// ==================================================

	tech = EffectMGR::HDRDownScaleE->GetTech("DownScaleSecond");
	tech->GetDesc(&techDesc);

	EffectMGR::HDRDownScaleE->SetResX(downSizeWidth);
	EffectMGR::HDRDownScaleE->SetResY(downSizeHeight);
	EffectMGR::HDRDownScaleE->SetDomain(downSizeWidth * downSizeHeight);
	EffectMGR::HDRDownScaleE->SetGroupSize(m_DownScaleGroups);
	// Output 
	EffectMGR::HDRDownScaleE->SetAvg1DUAV(m_pAvgLumUAV);
	//EffectMGR::HDRDownScaleE->SetDownScaleUAV(m_pDownScaleUAV);
	// Input
	EffectMGR::HDRDownScaleE->SetAvg1DSRV(m_pDownScale1DSRV);

	for (int p = 0; p < techDesc.Passes; p++)
	{
		tech->GetPassByIndex(p)->Apply(0, m_pDC);
		m_pDC->Dispatch(1, 1, 1);
	}

	m_pDC->CSSetShaderResources(0, 2, nullSRV);
	m_pDC->CSSetUnorderedAccessViews(0, 2, nullUAV, 0);

	m_pDC->CSSetShader(0, 0, 0);



}

void cBloomFilter::ToneMapping(ID3D11ShaderResourceView* pHDRSR)
{
	cCamera* cam = D3DMain::GetInstance()->GetCam();
	m_pDC->IASetInputLayout(InputLayOut::BasicInputLayout);
	m_pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	 XMMATRIX invProj = cam->GetProjMat();
	 XMVECTOR det =   XMMatrixDeterminant(invProj);
	 invProj = XMMatrixInverse(&det, invProj);

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
		EffectMGR::ToneMappingE->SetProjInv(invProj);

		EffectMGR::ToneMappingE->SetInputHDRSRV(pHDRSR);
		EffectMGR::ToneMappingE->SetAvgLumSRV(m_pAvgLumSRV);
		EffectMGR::ToneMappingE->SetBloomSRV(m_pBloomSRV);

		EffectMGR::ToneMappingE->SetMiddelGray(m_fMiddleGray);
		EffectMGR::ToneMappingE->SetWhiteSqrt(whiteSQRT);
		EffectMGR::ToneMappingE->SetBloomScale(m_fBloomScale);

		tech->GetPassByIndex(p)->Apply(0, m_pDC);
		m_pDC->DrawIndexed(6, 0, 0);
	}

	//m_pDC->VSSetShader(NULL, NULL, 0);
	//m_pDC->PSSetShader(NULL, NULL, 0);
	EffectMGR::ToneMappingE->SetInputHDRSRV(nullptr);

}



void cBloomFilter::BuildOffScreenOffSet()
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

void cBloomFilter::BuildResultTexture()
{
	// Down Scaled 된 Render Tartget 저장.
	D3D11_TEXTURE2D_DESC dtd = {
		m_Width, //UINT Width;
		m_Height, //UINT Height;
		1, //UINT MipLevels;
		1, //UINT ArraySize;
		DXGI_FORMAT_R16G16B16A16_FLOAT, //DXGI_FORMAT Format;
		1, //DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE ,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	};

	// Create the resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
	//	ZeroMemory(&dsrvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	dsrvd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	dsrvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	dsrvd.Texture2D.MostDetailedMip = 0;
	dsrvd.Texture2D.MipLevels = 1;

	ID3D11Texture2D* tex = 0;
	HR(m_pDevice->CreateTexture2D(&dtd, NULL, &tex));
	HR(m_pDevice->CreateShaderResourceView(tex, &dsrvd, &m_pResultSRV));
	HR(m_pDevice->CreateRenderTargetView(tex, 0, &m_pResultRTV));



}

void cBloomFilter::Blur(ID3D11ShaderResourceView* pInput, ID3D11UnorderedAccessView* pOutput, UINT blurCount)
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
			EffectMGR::BlurE->SetInputSRV(pInput);
			EffectMGR::BlurE->SetOutUAV(m_pTempUAV[1]);

			horz->GetPassByIndex(j)->Apply(0, m_pDC);

			UINT TreadCountX = (UINT)ceilf(m_Width / 256.0f);
			UINT TreadCountY = (UINT)ceil(m_Height / 1.0f);
			// 
			m_pDC->Dispatch(TreadCountX, TreadCountY, 1);
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
			EffectMGR::BlurE->SetInputSRV(m_pTempSRV[1]);
			EffectMGR::BlurE->SetOutUAV(pOutput);

			// 최종적으로  InputUAV에 모든 흐려진 결과가 나옴.

			vert->GetPassByIndex(j)->Apply(0, m_pDC);

			UINT TreadCountX = m_Width;
			UINT TreadCountY = (UINT)ceilf(m_Height / 256.0f);

			m_pDC->Dispatch(TreadCountX, TreadCountY, 1);
		}

		m_pDC->CSSetShaderResources(0, 1, srv);
		m_pDC->CSSetUnorderedAccessViews(0, 1, uav, 0);
	}
	// 계산 쉐이더를 비호라성화 한다.
	m_pDC->CSSetShader(0, 0, 0);
}


// 이것이 리얼 끄.
void cBloomFilter::Bloom()
{
	ID3D11UnorderedAccessView* nullUAV[2] = { nullptr, nullptr };
	ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };

	ID3DX11EffectTechnique* tech = EffectMGR::HDRDownScaleE->GetTech("BloomRevealFX");

	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc(&techDesc);

	UINT downSizeWidth = m_Width / 4;
	UINT downSizeHeight = m_Height / 4;

	EffectMGR::HDRDownScaleE->SetResX(downSizeWidth);
	EffectMGR::HDRDownScaleE->SetResY(downSizeHeight);
	EffectMGR::HDRDownScaleE->SetDomain(downSizeWidth * downSizeHeight);
	EffectMGR::HDRDownScaleE->SetGroupSize(m_DownScaleGroups);
	EffectMGR::HDRDownScaleE->SetBloomThreshold(m_fBloomThreshold);

	// Output 
	EffectMGR::HDRDownScaleE->SetOutputBloomUAV(m_pTempUAV[0]);
	// Input
	EffectMGR::HDRDownScaleE->SetBloomDownScaleSRV(m_pDownScaleSRV);
	EffectMGR::HDRDownScaleE->SetBloomAvgLumSRV(m_pAvgLumSRV);

	for (int p = 0; p < techDesc.Passes; p++)
	{
		tech->GetPassByIndex(p)->Apply(0, m_pDC);
		m_pDC->Dispatch(m_DownScaleGroups, 1, 1);
	}

	m_pDC->CSSetShaderResources(0, 2, nullSRV);
	m_pDC->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
	m_pDC->CSSetShader(0, 0, 0);
}






