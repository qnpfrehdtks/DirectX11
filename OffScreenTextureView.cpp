#include "DXUT.h"
#include "OffScreenTextureView.h"
#include "GeometryGenerator.h"
#include "Vertex.h"
#include "EffectMGR.h"
#include "Camera.h"
#include "Main.h"
#include "MathHelper.h"

cOffScreenTextureView::cOffScreenTextureView(ID3D11Device * device, ID3D11DeviceContext * dc, UINT width, UINT height)
	: m_pDevice(device), m_pDC(dc),
	m_OffScreenSRV(nullptr),
	m_OffScreenVB(nullptr), m_OffScreenIB(nullptr), 
	m_TexSRV(nullptr), m_TexUAV(nullptr),
	m_Width(width), m_Height(height)
	, m_OffsetIBCount(0), m_OffsetVBCount(0)
{
}

cOffScreenTextureView::~cOffScreenTextureView()
{
}

void cOffScreenTextureView::Init()
{
	// https://www.gamedev.net/forums/topic/669746-uses-for-unordered-access-views/
	// 순서 없는 접근 뷰가 무엇인지 이야기하는 외국글.
	// 출력 자원을 계산 쉐이더에 묶기 위해서는 순서 없는 접근 뷰가 필요 하다.
	// ID3D11UnorederedAccessView 인터페이스로 접근한다.
	SAFE_RELEASE(m_TexSRV);
	SAFE_RELEASE(m_TexUAV);

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
	ID3D11Texture2D* Tex = 0;
	HR(m_pDevice->CreateTexture2D(&tbd, 0, &Tex));

	// 쉐이더 자원 뷰로 생성하기 위해 구조체 서술.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	srvd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0;
	srvd.Texture2D.MipLevels = 1;

	// Shader 자원뷰 생성.
	HR(m_pDevice->CreateShaderResourceView(Tex, &srvd, &m_TexSRV));


	// 이번에는 계산 쉐이더의 출력을 위해, UAV로 묶어준다.
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
	uavd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavd.Texture2D.MipSlice = 0;
	// View 생성 후에는 Effect에 묶어 주어야 하는데...
	HR(m_pDevice->CreateUnorderedAccessView(Tex, &uavd, &m_TexUAV));

	BuildOffScreenView();
	BuildScreenGeometry();

	SAFE_RELEASE(Tex);
}

void cOffScreenTextureView::BuildOffScreenView()
{
	//SAFE_RELEASE(m_OffScreenRTV);
	//SAFE_RELEASE(m_OffScreenSRV);


	// 텍스처 재생성각띠~
	D3D11_TEXTURE2D_DESC texDesc;

	texDesc.Width = m_Width;
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
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* offscreenTex = 0;
	HR(m_pDevice->CreateTexture2D(&texDesc, 0, &offscreenTex));

	// Create the resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
	//	ZeroMemory(&dsrvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	dsrvd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	dsrvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	dsrvd.Texture2D.MostDetailedMip = 0;
	dsrvd.Texture2D.MipLevels = 1;

	// Texture를 쉐이더 자원 뷰, 입력으로도 넣고,
	HR(m_pDevice->CreateShaderResourceView(offscreenTex, &dsrvd, &m_OffScreenSRV));
	HR(m_pDevice->CreateRenderTargetView(offscreenTex, 0, &m_OffScreenRTV));
	// 계산 쉐이더에 기록할 자원을 묶을 UAV.

	// View saves a reference to the texture so we can release our reference.
	SAFE_RELEASE(offscreenTex);
}

void cOffScreenTextureView::BuildScreenGeometry()
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

void cOffScreenTextureView::SwapToOffScreen(ID3D11DepthStencilView* dsv)
{
	ID3D11RenderTargetView* rtv[1] = { m_OffScreenRTV };
	m_pDC->OMSetRenderTargets(1, rtv, dsv);

	m_pDC->ClearRenderTargetView(m_OffScreenRTV, Colors::AliceBlue);
	m_pDC->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void cOffScreenTextureView::DrawToScene()
{

	const cCamera* cam = D3DMain::GetInstance()->GetCam();
	//const sDirectionalLight* dirLight = LightMGR::GetInstance()->GetDirLights()[0].get();

	UINT stride = sizeof(sBasicVertex);
	UINT offset = 0;

	m_pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDC->IASetInputLayout(InputLayOut::BasicInputLayout);

	m_pDC->IASetVertexBuffers(0, 1, &m_OffScreenVB, &stride, &offset);
	m_pDC->IASetIndexBuffer(m_OffScreenIB, DXGI_FORMAT_R32_UINT, offset);

	XMMATRIX toTexSpace(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);


	// Set constants
	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX view = cam->GetViewMat();
	XMMATRIX proj = cam->GetProjMat();
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldInvTransposeViewProj = worldInvTranspose * view * proj;
	XMMATRIX worldViewProj = world * view * proj;

	// ===================== World, Proj, View... ===================
	EffectMGR::BasicE->SetWorld(world);
	EffectMGR::BasicE->SetViewProj(view * proj);
	EffectMGR::BasicE->SetWorldViewProj(worldViewProj);
	EffectMGR::BasicE->SetWorldViewProjTex(worldViewProj * toTexSpace);
//	EffectMGR::BasicE->(worldInvTransposeViewProj);
	EffectMGR::BasicE->SetWorldInvTranspose(worldInvTranspose);

	EffectMGR::BasicE->SetDiffuseMap(m_OffScreenSRV);
	//EffectMGR::BasicE->SetDirLights(&dirLight->dirLight);

	// =================== Dissolve ===========================

	//  ======================= Light ==========================


	EffectMGR::BasicE->SetEyePosW(cam->GetPos());

	EffectMGR::BasicE->SetTexTransform(XMMatrixIdentity());



	D3DX11_TECHNIQUE_DESC techDesc;

	ID3DX11EffectTechnique* tech;
	tech = EffectMGR::BasicE->GetTech("DirLight");

	tech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; p++)
	{
		float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		//m_pDC->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);

		tech->GetPassByIndex(p)->Apply(0, m_pDC);
		m_pDC->DrawIndexed(m_OffsetIBCount, 0, 0);
	}

	m_pDC->RSSetState(0);
	//	m_pDeviceContext->OMSetBlendState(0, 0, 0);
}

void cOffScreenTextureView::DrawToScene(ID3D11ShaderResourceView * rsv)
{

	const cCamera* cam = D3DMain::GetInstance()->GetCam();
	//const sDirectionalLight* dirLight = LightMGR::GetInstance()->GetDirLights()[0].get();

	UINT stride = sizeof(sBasicVertex);
	UINT offset = 0;

	m_pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDC->IASetInputLayout(InputLayOut::BasicInputLayout);

	m_pDC->IASetVertexBuffers(0, 1, &m_OffScreenVB, &stride, &offset);
	m_pDC->IASetIndexBuffer(m_OffScreenIB, DXGI_FORMAT_R32_UINT, offset);

	XMMATRIX toTexSpace(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);


	// Set constants
	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX view = cam->GetViewMat();
	XMMATRIX proj = cam->GetProjMat();
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldInvTransposeViewProj = worldInvTranspose * view * proj;
	XMMATRIX worldViewProj = world * view * proj;

	// ===================== World, Proj, View... ===================
	EffectMGR::BasicE->SetWorld(world);
	EffectMGR::BasicE->SetViewProj(view * proj);
	EffectMGR::BasicE->SetWorldViewProj(worldViewProj);
	EffectMGR::BasicE->SetWorldViewProjTex(worldViewProj * toTexSpace);
//	EffectMGR::BasicE->SetWorldInvTransposeViewProj(worldInvTransposeViewProj);
	EffectMGR::BasicE->SetWorldInvTranspose(worldInvTranspose);

	EffectMGR::BasicE->SetDiffuseMap(rsv);
	//EffectMGR::BasicE->SetDirLights(&dirLight->dirLight);

	// =================== Dissolve ===========================

	//  ======================= Light ==========================


	EffectMGR::BasicE->SetEyePosW(cam->GetPos());

	EffectMGR::BasicE->SetTexTransform(XMMatrixIdentity());



	D3DX11_TECHNIQUE_DESC techDesc;

	ID3DX11EffectTechnique* tech;
	tech = EffectMGR::BasicE->GetTech("DirLight");

	tech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; p++)
	{
		float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		//m_pDC->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);

		tech->GetPassByIndex(p)->Apply(0, m_pDC);
		m_pDC->DrawIndexed(m_OffsetIBCount, 0, 0);
	}

	m_pDC->RSSetState(0);
	//	m_pDeviceContext->OMSetBlendState(0, 0, 0);
}


