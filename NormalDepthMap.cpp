#include "DXUT.h"
#include "NormalDepthMap.h"
#include "cShape.h"
#include "Camera.h"
#include "EffectMGR.h"
#include "Vertex.h"
#include "MathHelper.h"


cNormalDepthMap::cNormalDepthMap(ID3D11Device * device, ID3D11DeviceContext * dc, UINT width, UINT height)
	: m_pDevice(device),
	m_pDeviceContext(dc),
	m_CountShape(0),
	m_NormalDepthDSV(nullptr),
	m_NormalDepthRTV(nullptr),
	m_NormalDepthSRV(nullptr),
	m_width(width), m_height(height)
{
	m_pShapeVector.reserve(12);
	Init();
}

cNormalDepthMap::~cNormalDepthMap()
{
	m_pShapeVector.clear();
	SAFE_RELEASE(m_NormalDepthDSV);
	SAFE_RELEASE(m_NormalDepthRTV);
	SAFE_RELEASE(m_NormalDepthSRV);
}

void cNormalDepthMap::InsertShapeToDraw(cBasicShape* shape)
{
	if (!shape) return;

	m_pShapeVector.push_back(shape);
	m_CountShape++;
}

void cNormalDepthMap::Init()
{

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = m_width;
	texDesc.Height = m_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* normalDepthTex = 0; // Shader Resouce View로도 Render Target 등으로 쓰인다.
	HR(m_pDevice->CreateTexture2D(&texDesc, 0, &normalDepthTex));
	// 텍스처 생성 후, SRV 그리고 , RTV로 묶어준다.
	HR(m_pDevice->CreateShaderResourceView(normalDepthTex, 0, &m_NormalDepthSRV));
	HR(m_pDevice->CreateRenderTargetView(normalDepthTex, 0, &m_NormalDepthRTV));

	SAFE_RELEASE(normalDepthTex);

}


void cNormalDepthMap::DrawToNormalDepth(cCamera* cam)
{

	ID3D11Buffer* VertexBuffer[1] = { nullptr };
	ID3D11Buffer* IndexBuffer = nullptr;

	XMMATRIX proj = cam->GetProjMat();
	XMMATRIX view = cam->GetViewMat();
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	ID3DX11EffectTechnique* tech = EffectMGR::NormalDepthE->GetTech("NormalDepth");

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldView;
	XMMATRIX worldInvTransposeView;
	XMMATRIX worldViewProj;

	UINT stride = sizeof(Vertexs::sBasicVertex);
	UINT offset = 0;

	m_pDeviceContext->IASetInputLayout(InputLayOut::BasicInputLayout);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);

	for (int p = 0; p < techDesc.Passes; p++)
	{

		for (int i = 0; i < m_CountShape; i++)
		{
			VertexBuffer[0] =  m_pShapeVector[i]->GetVertexBuffer();
			IndexBuffer = m_pShapeVector[i]->GetIndexBuffer();

			m_pDeviceContext->IASetVertexBuffers(0, 1, VertexBuffer, &stride, &offset);
			m_pDeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, offset);

			world = XMLoadFloat4x4(&m_pShapeVector[i]->GetWorld());
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldView = world * view;
			worldInvTransposeView = worldInvTranspose * view;
			worldViewProj = world * view * proj;

			EffectMGR::NormalDepthE->SetTexTransform(XMMatrixIdentity());
			EffectMGR::NormalDepthE->SetWorldViewInvTranspose(worldInvTransposeView);
			EffectMGR::NormalDepthE->SetWorldInvTranspose(worldInvTranspose);
			EffectMGR::NormalDepthE->SetWorldViewProj(worldViewProj);
			EffectMGR::NormalDepthE->SetWorldView(worldView);
			EffectMGR::NormalDepthE->SetWorld(world);

			tech->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
			m_pDeviceContext->DrawIndexed(m_pShapeVector[i]->GetIdxCount(), 0, 0);
		}
	}


}

void cNormalDepthMap::SwapToNormalMapDepthRTV(ID3D11DepthStencilView* dsv)
{
	// Render Target을 바꾸어주는 메소드, 깊이 스텐실은 기존의 것을 이용하고
	// Normal Depth를 그릴 RTV 로 변경 시켜준다. 
	ID3D11RenderTargetView* renderTargets[1] = { m_NormalDepthRTV };
	m_pDeviceContext->OMSetRenderTargets(1, renderTargets, dsv);

	float color[] = { 0.0f,0.0f,-1.0f,1e5f };
	// RTV Clear 해준다.
	m_pDeviceContext->ClearRenderTargetView(m_NormalDepthRTV, color);
}

void cNormalDepthMap::BuildFrustumFarCorners(float fovy, float farZ)
{
	// Render Target View의 가로, 높이의 비
	float aspect = (float)800 / (float)600;

	float halfHeight = tanf(0.5* fovy) * farZ;
	float halfWidth = halfHeight * aspect;

	// 1----------2
	// |          |
	// |          |
	// |          |
	// |          |
	// 0----------3
	m_FrustumCorner[0] = XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	m_FrustumCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	m_FrustumCorner[2] = XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);
	m_FrustumCorner[3] = XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);

}

