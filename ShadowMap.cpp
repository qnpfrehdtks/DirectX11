#include "DXUT.h"
#include "ShadowMap.h"
#include <DirectXMath.h>
#include "LightMGR.h"
#include "EffectMGR.h"
#include "Vertex.h"
#include "cShape.h"
#include "Main.h"
#include "Camera.h"
#include "MathHelper.h"

using namespace Vertexs;
using namespace DirectX;


cShadowMap::cShadowMap(ID3D11Device * device, ID3D11DeviceContext * dc, float width, float height)
	: m_pDevice(device), m_pDeviceContext(dc), m_width(width), m_height(height)
{
}

cShadowMap::~cShadowMap()
{
	SAFE_RELEASE(m_RTV);
	SAFE_RELEASE(m_DSV);
	SAFE_RELEASE(m_SRV);
}

void cShadowMap::Init()
{
	CreateShadowTexture(m_pDevice, m_pDeviceContext);
	BuildShadowTransform();
}

void cShadowMap::UpdateShadow(float dt)
{
	BuildShadowTransform();
}

void cShadowMap::CreateShadowTexture(ID3D11Device* device, ID3D11DeviceContext* dc)
{
	m_center = XMFLOAT3(0, 0.0f, 0);
	m_radius = sqrtf(200.0f*400.0f + 200.0f*400.0f);

	m_ViewPort.TopLeftX = 0.0f;
	m_ViewPort.TopLeftY = 0.0f;
	m_ViewPort.Height = static_cast<float>(m_height);
	m_ViewPort.Width = static_cast<float>(m_width);
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = m_width;
	texDesc.Height = m_height;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* depthMap = 0;
	HR(device->CreateTexture2D(&texDesc, 0, &depthMap));

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(device->CreateDepthStencilView(depthMap, &dsvDesc, &m_DSV));


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	HR(device->CreateShaderResourceView(depthMap, &srvDesc, &m_SRV));


	SAFE_RELEASE(depthMap);

}


// Light는 지속적으로 변하기 때문에, 나는 여기서 , 빛의  View 시점에 따라 변수를 저장하고, 
// Light 시점의 View Space 변환 행렬을 만든다.
void cShadowMap::BuildShadowTransform()
{
	XMFLOAT3 dir = ( *LightMGR::GetInstance()->GetDirLights() )[0].Direction;
	XMVECTOR LightDir = XMLoadFloat3(&dir);
	XMVECTOR LightPos = -2.0f * m_radius * LightDir;
	XMVECTOR targetPos = XMLoadFloat3(&m_center);

	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX V = XMMatrixLookAtLH(LightPos, targetPos, up);

	XMFLOAT3 sphereCenterLS;

	// 바라보는 지점을 View Mat와 곱해서, 빛의 시야 공간으로 변환한다.
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

	// 시야 공간 구성.
	float l = sphereCenterLS.x - m_radius;
	float b = sphereCenterLS.y - m_radius;
	float n = sphereCenterLS.z - m_radius;
	float r = sphereCenterLS.x + m_radius;
	float t = sphereCenterLS.y + m_radius;
	float f = sphereCenterLS.z + m_radius;

	// 장면을 감싸는 직교투영 상자 행렬을 정의함.
	XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
	// 텍스처 변환 행렬.
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	// 투영 텍스처 변환 행렬 전개.
	XMMATRIX S = V * P * T;
	// 
	XMStoreFloat4x4(&m_LightView, V);
	XMStoreFloat4x4(&m_LightProj, P);
	XMStoreFloat4x4(&m_ShadowTransform, S);
}

// 그림자 매핑을 위해 그림자 Map 텍스처로 Swap해서 그림자 깊이 맵을 그려주자.
void cShadowMap::SwapRenderTargetToShadow()
{
	m_pDeviceContext->RSSetViewports(1, &m_ViewPort);

	ID3D11RenderTargetView* renderTargets[1] = { nullptr };

	// 렌더 대상 비활성화. -> 색상 쓰기 비활성화
	// 깊이 버퍼에만 그리기 위해서

	m_pDeviceContext->OMSetRenderTargets(1, renderTargets, m_DSV);
	m_pDeviceContext->ClearDepthStencilView(m_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

}


void cShadowMap::DrawToShadowMap(std::vector<cBasicShape*>& shapes, cCamera* cam, UINT count)
{
	m_pDeviceContext->RSSetState(0);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetInputLayout(InputLayOut::BasicInputLayout);

	UINT stride = sizeof(sBasicVertex);
	UINT offset = 0;

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX WVP;

	XMMATRIX view = XMLoadFloat4x4(&m_LightView);
	XMMATRIX proj = XMLoadFloat4x4(&m_LightProj);
	XMMATRIX VP = XMMatrixMultiply(view, proj);

	EffectMGR::ShadowE->SetViewProj(VP);
	EffectMGR::ShadowE->SetEyePosW(cam->GetPos());

	EffectMGR::ShadowE->SetMaxTessFactor(5.0f);
	EffectMGR::ShadowE->SetMinTessFactor(1.0f);
	EffectMGR::ShadowE->SetMaxTessDistance(1.0f);
	EffectMGR::ShadowE->SetMinTessDistance(25.0f);
	EffectMGR::ShadowE->SetHeightScale(0.07f);

	D3DX11_TECHNIQUE_DESC techDesc;
	ID3DX11EffectTechnique* smapTech = EffectMGR::ShadowE->GetTech("BuildShadowMapTech");

	for (int i = 0; i < m_Shapes.size(); i++)
	{
		smapTech->GetDesc(&techDesc);

	     for (int p = 0; p < techDesc.Passes; p++)
	     {
			ID3D11Buffer* VBuffer = m_Shapes[i]->GetVertexBuffer();

			m_pDeviceContext->IASetVertexBuffers(0, 1, &VBuffer, &stride, &offset);
			m_pDeviceContext->IASetIndexBuffer(m_Shapes[i]->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, offset);
			
		    world = XMLoadFloat4x4(&(m_Shapes[i]->GetWorld()));
			//world = XMMatrixIdentity();
			worldInvTranspose = MathHelper::InverseTranspose(world);
			WVP = world * view * proj;

			EffectMGR::ShadowE->SetWorld(world);
			EffectMGR::ShadowE->SetWorldInvTranspose(worldInvTranspose);
			EffectMGR::ShadowE->SetWorldViewProj(WVP);
			EffectMGR::ShadowE->SetTexTransform(XMMatrixIdentity());

			smapTech->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
			m_pDeviceContext->DrawIndexed(m_Shapes[i]->GetIdxCount(), 0, 0);
		}
	}

}

void cShadowMap::DrawToShadowMap(sSkinModellInstance & skinModel, cCamera * cam, UINT count, BOOL hasAni)
{
	m_pDeviceContext->RSSetState(0);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetInputLayout(InputLayOut::SkinInputLayout);

	UINT stride = sizeof(sSkinnedVertex);
	UINT offset = 0;

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX WVP;

	XMMATRIX view = XMLoadFloat4x4(&m_LightView);
	XMMATRIX proj = XMLoadFloat4x4(&m_LightProj);
	XMMATRIX VP = XMMatrixMultiply(view, proj);

	EffectMGR::ShadowE->SetViewProj(VP);
	EffectMGR::ShadowE->SetEyePosW(cam->GetPos());

	EffectMGR::ShadowE->SetMaxTessFactor(5.0f);
	EffectMGR::ShadowE->SetMinTessFactor(1.0f);
	EffectMGR::ShadowE->SetMaxTessDistance(1.0f);
	EffectMGR::ShadowE->SetMinTessDistance(25.0f);
	EffectMGR::ShadowE->SetHeightScale(0.07f);

	D3DX11_TECHNIQUE_DESC techDesc;
	ID3DX11EffectTechnique* smapTech;

	if(!hasAni)
	 smapTech = EffectMGR::ShadowE->GetTech("BuildShadowMapNoSkinTech");
	else   smapTech = EffectMGR::ShadowE->GetTech("BuildShadowMapSkinTech");

	UINT meshCount = skinModel.Model->GetMeshCount();

	for (UINT meshID = 0; meshID < meshCount; meshID++)
	{

		if (meshID == 4) continue;

		smapTech->GetDesc(&techDesc);

		for (UINT p = 0; p < techDesc.Passes; p++)
		{
			ID3D11Buffer* pIBuffer = skinModel.Model->GetIndiceBuffer(meshID);
			ID3D11Buffer* pVBuffer = skinModel.Model->GetVertexBuffer(meshID);

			m_pDeviceContext->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
			m_pDeviceContext->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R16_UINT, offset);

			world = XMLoadFloat4x4(&(skinModel.World));
			worldInvTranspose = MathHelper::InverseTranspose(world);
			WVP = world * view * proj;

			EffectMGR::ShadowE->SetWorld(world);
			EffectMGR::ShadowE->SetWorldInvTranspose(worldInvTranspose);
			EffectMGR::ShadowE->SetWorldViewProj(WVP);
			EffectMGR::ShadowE->SetTexTransform(XMMatrixIdentity());
			if (hasAni)
			EffectMGR::ShadowE->SetBoneTransform(&skinModel.FinalTransforms[0], skinModel.FinalTransforms.size());

			smapTech->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
			m_pDeviceContext->DrawIndexed(skinModel.Model->GetIndiceBufferCount(meshID), 0, 0);
		}
		//}
	}
}

void cShadowMap::InsertShape(cBasicShape * shape)
{
	if (!shape) return;
	m_Shapes.push_back(shape);
}
