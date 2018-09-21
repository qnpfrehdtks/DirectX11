#include "DXUT.h"
#include "DeferredShading.h"
#include "EffectMGR.h"
#include "Vertex.h"
#include "Camera.h"
#include "Main.h"
#include "Gbuffer.h"
#include "MathHelper.h"
#include "RenderState.h"
#include "LightMGR.h"
#include "ShadowMap.h"

DeferredShading::DeferredShading()
{
}


DeferredShading::~DeferredShading()
{
}

void DeferredShading::BuildFrustumCorners(float fov, float farZ)
{
	cCamera* cam = D3DMain::GetInstance()->GetCam();

	A = cam->GetFarZ() / (cam->GetFarZ() - cam->GetNearZ());
	B = (-cam->GetNearZ() / (cam->GetFarZ() - cam->GetNearZ()));

	float aspect = D3DMain::GetInstance()->GetWidth() / D3DMain::GetInstance()->GetHeight();
	float halfHeight =  tanf(fov * 0.5f) * farZ;
	float halfWidth = aspect * halfHeight;

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

void DeferredShading::Init(ID3D11Device * dc, ID3D11DeviceContext * pDC)
{
	const cCamera* cam = D3DMain::GetInstance()->GetCam();

	BuildFrustumCorners(cam->GetFovY(), cam->GetFarZ());
	BuildScreenQuad(dc);

}

void DeferredShading::LightPass(ID3D11DeviceContext * pDC, cGBuffer* Gbuffer, cCamera* cam, LPCSTR str, cShadowMap* shadowMap, ID3D11ShaderResourceView* ssaoMap)
{
	 sDirLight dirLight = (*LightMGR::GetInstance()->GetDirLights())[0];

	sPointLight ptLights;
	std::vector<sPtLight> pts = (*LightMGR::GetInstance()->GetPtLights());


	UINT stride = sizeof(sBasicVertex);
	UINT offset = 0;

	XMMATRIX P = cam->GetProjMat();
	XMMATRIX V = cam->GetViewMat();
	XMMATRIX VP = V * P;


	  XMMatrixDeterminant(V);
	// XMMATRIX InvV = XMMatrixInverse(nullptr, V);
	 XMMATRIX InvVP = XMMatrixInverse(nullptr, VP);
	 XMMATRIX InvP = XMMatrixInverse(nullptr, P);

	 XMFLOAT4X4 pmat;
	 XMStoreFloat4x4(&pmat, P);

	 XMFLOAT4 ProjProperty = { pmat._11,pmat._12 , A, B };

	 XMFLOAT3 CamPos = cam->GetPos();

	 EffectMGR::DeferredShadingE->SetDepthMap(Gbuffer->GetDepthSRV());
	 EffectMGR::DeferredShadingE->SetDiffuseSpecIntMap(Gbuffer->GetDiffuseSRV());
	 EffectMGR::DeferredShadingE->SetNormalMap(Gbuffer->GetNormalSRV());
	 EffectMGR::DeferredShadingE->SetSpecPowMap(Gbuffer->GetSpecPowerSRV());

	 EffectMGR::DeferredShadingE->SetWorldViewProj(XMMatrixIdentity());
	 EffectMGR::DeferredShadingE->SetEyePosW(D3DMain::GetInstance()->GetCam()->GetPos());
	 EffectMGR::DeferredShadingE->SetFrustumCorners(m_FrustumCorner);
	 EffectMGR::DeferredShadingE->SetProjProperty(ProjProperty);
	
	 EffectMGR::DeferredShadingE->SetDirLights(&dirLight);
	 EffectMGR::DeferredShadingE->SetPtLights(&pts[0],800);
	 EffectMGR::DeferredShadingE->SetInvProj(InvP);
	 EffectMGR::DeferredShadingE->SetInvViewProj(InvVP);

	 EffectMGR::DeferredShadingE->SetSSAOMap(ssaoMap);

	 EffectMGR::DeferredShadingE->SetShadowMap(shadowMap->GetShadowMap());
	 EffectMGR::DeferredShadingE->SetShadowTransform( XMLoadFloat4x4(&shadowMap->GetShadowTransform()));

	// EffectMGR::DeferredShadingE->SetInvViewProj(InvVP);

	 pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	 pDC->IASetInputLayout(InputLayOut::BasicInputLayout);
	 pDC->IASetVertexBuffers(0, 1, &m_VB, &stride, &offset);
	 pDC->IASetIndexBuffer(m_IB, DXGI_FORMAT_R16_UINT, offset);

	 ID3DX11EffectTechnique* tech = EffectMGR::DeferredShadingE->GetTech(str);
	 D3DX11_TECHNIQUE_DESC techDesc;
	 tech->GetDesc(&techDesc);

	 for (UINT p = 0; p < techDesc.Passes; ++p)
	 {
		 tech->GetPassByIndex(p)->Apply(0, pDC);
		 pDC->DrawIndexed(6, 0, 0);
	 }

}

void DeferredShading::BuildScreenQuad(ID3D11Device* pDevice)
{
	//  Far Plane을 만들기 위한 메소드
	sBasicVertex v[4];

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
	vbd.ByteWidth = sizeof(sBasicVertex) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;

	// Buffer 생성
	HR(pDevice->CreateBuffer(&vbd, &vinitData, &m_VB));

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

	HR(pDevice->CreateBuffer(&ibd, &iinitData, &m_IB));
}
