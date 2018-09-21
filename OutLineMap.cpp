#include "DXUT.h"
#include "OutLineMap.h"
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


cOutLineMap::cOutLineMap(ID3D11Device * device, ID3D11DeviceContext * dc, float width, float height)
	: m_pDevice(device), m_pDeviceContext(dc), m_width(width), m_height(height)
{
}

cOutLineMap::~cOutLineMap()
{
	SAFE_RELEASE(m_SilhouetteEdgeSRV);
	SAFE_RELEASE(m_SilhouetteEdgeRTV);
}

void cOutLineMap::Init()
{
	m_NormalDepthMap = new cNormalDepthMap(m_pDevice, m_pDeviceContext, m_width, m_height);
	OnSize();
	CreateOutLineTexture(m_pDevice, m_pDeviceContext);
	
	// Normal Depth Map �� ����� Shader Resource View�� �Է����� ����, ����ϱ� ���� Screen Quad View. 
	BuildScreenQuad();
	BuildOffsetVector();
//	BuildShadowTransform();
}

void cOutLineMap::OnSize()
{
	m_ViewPort.Width = m_width;
	m_ViewPort.Height = m_height;
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;
	m_ViewPort.TopLeftX = 0.0f;
	m_ViewPort.TopLeftY = 0.0f;
}

void cOutLineMap::BuildScreenQuad()
{
	//  Far Plane�� ����� ���� �޼ҵ�
	Vertexs::sBasicVertex v[4];

	v[0].Pos = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	v[1].Pos = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	v[2].Pos = XMFLOAT3(1.0f, 1.0f, 0.0f);
	v[3].Pos = XMFLOAT3(1.0f, -1.0f, 0.0f);

	// Store far plane frustum corner indices in Normal.x slot.
	v[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[1].Normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
	v[2].Normal = XMFLOAT3(2.0f, 0.0f, 0.0f);
	v[3].Normal = XMFLOAT3(3.0f, 0.0f, 0.0f);

	v[0].Tex = XMFLOAT2(0.0f, 1.0f);
	v[1].Tex = XMFLOAT2(0.0f, 0.0f);
	v[2].Tex = XMFLOAT2(1.0f, 0.0f);
	v[3].Tex = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertexs::sBasicVertex) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;

	// Buffer ����
	HR(m_pDevice->CreateBuffer(&vbd, &vinitData, &m_SrcreenQuadVB));

	USHORT indices[6] =
	{
		0,1,2,
		0,2,3
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * 6;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;

	HR(m_pDevice->CreateBuffer(&ibd, &iinitData, &m_SrcreenQuadIB));
}



void cOutLineMap::CreateOutLineTexture(ID3D11Device* device, ID3D11DeviceContext* dc)
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

	ID3D11Texture2D* OutLineTex = 0; // Shader Resouce View�ε� Render Target ������ ���δ�.
	HR(m_pDevice->CreateTexture2D(&texDesc, 0, &OutLineTex));
	// �ؽ�ó ���� ��, SRV �׸��� , RTV�� �����ش�.
	HR(m_pDevice->CreateShaderResourceView(OutLineTex, 0, &m_SilhouetteEdgeSRV));
	HR(m_pDevice->CreateRenderTargetView(OutLineTex, 0, &m_SilhouetteEdgeRTV));

	SAFE_RELEASE(OutLineTex);

	// SSAO �� ���� Texture�� ���� ũ���� 1/4 ũ��� �ٿ��� ��������.
	// ũ�⸦ ���̰�, Format ������ FLOAT ��������
	//texDesc.Format = DXGI_FORMAT_R16_FLOAT;
	//ID3D11Texture2D* ambiantTex0 = 0;


}


void cOutLineMap::Update(float dt)
{
	BuildOutLineTransform();
}

//void cOutLineMap::SwapRenderTargetToOutLineMap()
//{
//	ID3D11RenderTargetView* renderTargets[1] = { m_RTV };
//
//	// ���� ��� ��Ȱ��ȭ. -> ���� ���� ��Ȱ��ȭ
//	// ���� ���ۿ��� �׸��� ���ؼ�
//	m_pDeviceContext->OMSetRenderTargets(1, renderTargets, D3DMain::GetInstance()->GetDSV());
//	m_pDeviceContext->ClearRenderTargetView(m_RTV, reinterpret_cast<const float*>(&Colors::Silver));
//
//}


void cOutLineMap::DrawToNormalDepthMap(cCamera * cam )
{
	m_NormalDepthMap->SwapToNormalMapDepthRTV(D3DMain::GetInstance()->GetDSV());
	m_NormalDepthMap->DrawToNormalDepth(cam);


	// *********** ���⿡ OutLine  ����. *************//
}


// ���������� Normal, Depth Map�� ������ �ܰ������� ����ϰ� �׸�.
void cOutLineMap::ComputeOutLineMap(cCamera * cam)
{
	ID3D11RenderTargetView* renderTargets[1] = { m_SilhouetteEdgeRTV };
	// Amient Map�� ���� Ÿ������ ����, ���� ���ٽ��� ���� �ʴ´�.
	m_pDeviceContext->OMSetRenderTargets(1, renderTargets, 0);

	// �� ������ ���� RTV�� �ʱ�ȭ �����ش�.
	m_pDeviceContext->ClearRenderTargetView(m_SilhouetteEdgeRTV, reinterpret_cast<const float*>(&Colors::Aqua));
	// ViewPort ����.
	m_pDeviceContext->RSSetViewports(1, &m_ViewPort);

	// q : ������ �ش� �ȼ� �� ������ ������ ǥ����. 
	// �� q����, ī�޶� �������� �� ���� �ؽ�ó ��ǥ�� ���ؼ�,
	// ����,���� ���� ǥ���� �����ϰ�, �� ǥ���� �̿��ؼ� q�� ���� �������� ���� ����� ���� �ȼ�
	// r ���� ���� �� �ִ�. 
	// �׷��� �켱 ���� �ؽ�ó ����� ���ϵ��� ����.

	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX P = cam->GetProjMat();
	XMMATRIX PT = XMMatrixMultiply(P, T);

	UINT stride = sizeof(Vertexs::sBasicVertex);
	UINT offset = 0;

	// ��ü ȭ�� �簢���� �׸� ��, �簢���� �� �������� ���� �� ������ �ش��ϴ� �� ��� ����ü �������� ���� ���� ����.
	EffectMGR::OutLineE->SetFrustumCorners(m_NormalDepthMap->GetFrustumFarCorner());
	EffectMGR::OutLineE->SetNormalDepthMap(m_NormalDepthMap->GetDepthSRV());  // �տ��� ���� Normal, Depth Texture�� �����ش�.
	EffectMGR::OutLineE->SetViewToTexSpace(PT); // View Space ���� Texure Space�� ��ȯ�� ���. //
	EffectMGR::OutLineE->SetOffsetVectors(m_Offset);

	// Effect FX �� ������ �Ѱ� ������ ����, DC�� ����, IA�� ����, �ε������� �ѱ�.
	m_pDeviceContext->IASetInputLayout(InputLayOut::BasicInputLayout);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_SrcreenQuadVB, &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(m_SrcreenQuadIB, DXGI_FORMAT_R16_UINT, 0);

	ID3DX11EffectTechnique* tech = EffectMGR::OutLineE->GetTech("SilhouetteEdge");
	D3DX11_TECHNIQUE_DESC desc;
	tech->GetDesc(&desc);

	for (int p = 0; p < desc.Passes; p++)
	{
		tech->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
		m_pDeviceContext->DrawIndexed(6, 0, 0);
	}
}

void cOutLineMap::InsertShape(cBasicShape * shape)
{
	m_NormalDepthMap->InsertShapeToDraw(shape);
}


void cOutLineMap::BuildOutLineTransform()
{
	cCamera* cam = D3DMain::GetInstance()->GetCam();

	auto V = cam->GetViewMat();
	auto P = cam->GetProjMat();

	
	// �ؽ�ó ��ȯ ���.
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	// ���� �ؽ�ó ��ȯ ��� ����.
	XMMATRIX S = V * P * T;
	XMStoreFloat4x4(&m_OutLineTransform, S);
}

void cOutLineMap::BuildOffsetVector()
{
	// 8 cube corners
	m_Offset[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	m_Offset[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	m_Offset[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	m_Offset[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	m_Offset[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	m_Offset[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	m_Offset[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	m_Offset[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 centers of cube faces
	m_Offset[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	m_Offset[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	m_Offset[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	m_Offset[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	m_Offset[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	m_Offset[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (int i = 0; i < 14; ++i)
	{
		// Create random lengths in [0.25, 1.0].
		float s = MathHelper::RandF(1.0f, 1.0f);

		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&m_Offset[i]));

		XMStoreFloat4(&m_Offset[i], v);
	}
}



//void cOutLineMap::Dra(std::vector<cBasicShape*>& shapes, cCamera * cam, UINT count)
//{
//	m_pDeviceContext->RSSetState(0);
//	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	m_pDeviceContext->IASetInputLayout(InputLayOut::BasicInputLayout);
//
//	UINT stride = sizeof(sBasicVertex);
//	UINT offset = 0;
//
//	XMMATRIX world = XMMatrixIdentity();
//	XMMATRIX WVPInvTranspose;
//	XMMATRIX view = cam->GetViewMat();
//	XMMATRIX proj = cam->GetProjMat();
//	XMMATRIX VP = XMMatrixMultiply(view, proj);
//	XMMATRIX WVP;
//
//	D3DX11_TECHNIQUE_DESC techDesc;
//	ID3DX11EffectTechnique* omapTech = EffectMGR::OutLineE->GetTech("OutLineTech");
//	omapTech->GetDesc(&techDesc);
//
//	ID3D11Buffer* VBuffer = shapes[0]->GetVertexBuffer();
//
//	m_pDeviceContext->IASetVertexBuffers(0, 1, &VBuffer, &stride, &offset);
//	m_pDeviceContext->IASetIndexBuffer(shapes[0]->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
//
//	for (int p = 0; p < techDesc.Passes; p++)
//	{
//
//		for (int i = 0; i < count; i++)
//		{
//			world = XMLoadFloat4x4(&shapes[i]->GetWorld());
//			WVP = world * view * proj;
//			WVPInvTranspose = MathHelper::InverseTranspose(WVP);
//
//			EffectMGR::OutLineE->SetWorldViewProj(WVP);
//			EffectMGR::OutLineE->SetTexTransform(XMMatrixIdentity());
//			EffectMGR::OutLineE->SetWorldViewProjInvTranspose(WVPInvTranspose * VP);
//
//			omapTech->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
//			m_pDeviceContext->DrawIndexed(shapes[i]->GetIdxCount(), 0, 0);
//		}
//	}
//}
